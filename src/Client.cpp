#include "Client.hpp"

#include <cstdio>
#include <optional>

#include <emscripten.h>
#include <emscripten/html5.h>
#include "util/Bucket.hpp"
#include "util/emsc/audio.hpp"
#include "util/emsc/jswebsockets.hpp"
#include "util/emsc/request.hpp"

#include "JsApiProxy.hpp"
#include "PacketDefinitions.hpp"
#include "util/misc.hpp"
#include "world/World.hpp"

#if __has_feature(address_sanitizer)
#	include <sanitizer/lsan_interface.h>

EM_JS(void*, get_evt_pointer, (const char* buf), {
	var s = UTF8ToString(buf);
	return JSEvents[s] ? JSEvents[s] : 0;
});
#endif

// TODO: Translate strings

EM_JS(void, set_client_status, (const char* buf, std::size_t len), {
	var el = document.getElementById("status");
	el.innerHTML = UTF8ToString(buf, len);
});

EM_JS(void, set_loadscreen_visible, (bool s), {
	var el = document.getElementById("loader");
	if (s) {
		el.classList.remove("ok");
	} else {
		el.classList.add("ok");
	}
});

EM_JS(void, set_login_prompt_visible, (bool s), {
	var el = document.getElementById("login");
	if (s) {
		el.classList.add("show");
	} else {
		el.classList.remove("show");
	}
});

static void checkHttpSession(void (*cb)(bool)) {
	async_request(
		"/api/identified", "GET", nullptr, reinterpret_cast<void*>(cb), true,
		[](unsigned, void* e, void* vbuf, unsigned len) { // ok
			void (*cb)(bool) = reinterpret_cast<void (*)(bool)>(e);
			const char* buf = static_cast<const char*>(vbuf);

			cb(len == 1 && buf[0] == '1');
		},
		[](unsigned, void* e, int code, const char* err) { // fail
			void (*cb)(bool) = reinterpret_cast<void (*)(bool)>(e);

			cb(false);
		},
		nullptr
	);
}

Client::Client(JsApiProxy& api)
: api(api),
  im("#input"),
  aClient(im.mkAdapter("Client", -1)),
#if __has_feature(address_sanitizer)
  iDoLeakCheck(aClient, "Leak check", T_ONPRESS),
#endif
  selfUid(0),
  tickTimer(emscripten_set_interval(Client::doTick, 1000.0 / Client::ticksPerSec, this)),
  lastError(CE_NONE) {
	std::printf("[Client] Created\n");
	js_ws_on_open(Client::doWsOpen);
	js_ws_on_close(Client::doWsClose);
	js_ws_on_message(Client::doWsMessage);
	js_ws_set_user_data(this);

	api.setClientInstance(this);
	registerPacketTypes();

#if __has_feature(address_sanitizer)
	iDoLeakCheck.setDefaultKeybind("O");
	iDoLeakCheck.setCb([](ImAction::Event&, const InputInfo&) {
		for (const char* s : {"touchEvent", "focusEvent", "wheelEvent", "mouseEvent", "uiEvent"}) {
			if (void* p = get_evt_pointer(s)) {
				__lsan_ignore_object(p);
			}
		}

		__lsan_do_recoverable_leak_check();
		return false;
	});
#endif

	skAudioEnableCh = Settings::get().enableAudio.connect([](auto b) { setAudioEnabled(b); });

	skJoinVolCh = Settings::get().joinSfxVol.connect([](auto v) { setVolumeAudioId("a-join", v); });

	skButtonVolCh = Settings::get().buttonSfxVol.connect([](auto v) { setVolumeAudioId("a-btn", v); });

	skPaintVolCh = Settings::get().paintSfxVol.connect([](auto v) { setVolumeAudioId("a-pixel", v); });
}

Client::~Client() {
	setStatus("Unloading...");

	if (js_ws_get_ready_state() != EWsReadyState::CLOSED) {
		js_ws_close(4001);
	}

	js_ws_set_user_data(nullptr); // prev ptr won't be valid when i return
	api.setClientInstance(nullptr);
	emscripten_clear_interval(tickTimer);
	std::puts("[Client] Destroyed");
}

bool Client::open(std::string url, std::string_view worldToJoin) {
	setStatus("Connecting...");

	url += '/';
	url += worldToJoin;

	return js_ws_open(url.data(), url.size(), "OWOP", 4);
}

bool Client::reconnect() {
	return js_ws_reconnect();
}

void Client::close() {
	js_ws_close(4000);
}

void Client::send(std::unique_ptr<u8[]> buf, std::size_t len) {
	js_ws_send(reinterpret_cast<char*>(buf.get()), len);
}

void Client::send(std::tuple<std::unique_ptr<u8[]>, std::size_t> pkt) {
	auto& [buf, len] = pkt;
	send(std::move(buf), len);
}

bool Client::freeMemory() {
	if (world && (world->freeMemory() || world->freeMemory(true))) {
		return true;
	}

	std::puts("[Client] Couldn't free any memory. Fasten your seatbelts.");
	return false;
}

void Client::setStatus(std::string_view s) {
	set_client_status(s.data(), s.size());
}

void Client::registerPacketTypes() {
	// pr.on<CAuthProgress>([](std::string currentProcessor) {
	// 	setStatus("Authenticating... (" + currentProcessor + ")");
	// 	std::printf("AuthProgress: %s\n", currentProcessor.c_str());
	// });

	pr.on<CAuthOk>([this](net::UviasUser usr) {
		auto [_selfUid, username, totRep, rid, rankName, isSuperUser, canSelfManage] = usr;
		setStatus("Joining world...");

		std::printf(
			"AuthOk: Uid=%llX Username=%s TotalRep=%i RankId=%u RankName=%s SuperUser=%u CanSelfManage=%u\n", _selfUid,
			username.c_str(), totRep, rid, rankName.c_str(), isSuperUser, canSelfManage
		);

		selfUid = _selfUid;
		users.try_emplace(
			selfUid, selfUid, totRep, UviasRank(rid, std::move(rankName), isSuperUser, canSelfManage),
			std::move(username)
		);
	});

	// pr.on<CAuthError>([this](std::string processor) {
	// 	setStatus("Auth error: " + processor);
	// 	std::printf("AuthError: %s\n", processor.c_str());

	// 	if (processor == "SessionChecker") {
	// 		lastError = CE_SESSION;
	// 	} else if (processor == "BanChecker") {
	// 		lastError = CE_BAN;
	// 	} else if (processor == "WorldChecker") {
	// 		lastError = CE_WORLD;
	// 	} else if (processor == "HeaderChecker") {
	// 		lastError = CE_HEADER;
	// 	} else if (processor == "ProxyChecker") {
	// 		lastError = CE_PROXY;
	// 	} else if (processor == "CaptchaChecker") {
	// 		lastError = CE_CAPTCHA;
	// 	} else {
	// 		lastError = CE_NONE;
	// 	}
	// });

	pr.on<CPlayerData>([this](net::PlayerUpd<net::DAbsWPos> selfCur, net::Bucket action, net::Bucket chat, bool canChat, bool canPaint, net::DStateSyncSeq sseq, net::DActionSyncSeq aseq) {
		auto [cid, x, y, step, tid, tstate] = selfCur;
		auto [arate, aper, aallowance] = action;
		auto [crate, cper, callowance] = chat;
		Bucket actionBkt(arate, aper, aallowance);
		Bucket chatBkt(crate, cper, callowance);

		std::printf(
			"CPlayerData: ID=%llu X=%lld Y=%lld Step=%u ToolID=%u ABucketRate=%u ABucketPer=%u ABucketAllowance=%f "
			"CBucketRate=%u CBucketPer=%u CBucketAllowance=%f CanChat=%u CanPaint=%u StateSeq=%u ActionSeq=%u\n",
			cid.get(), x.get(), y.get(), step, tid, arate, aper, aallowance, crate, cper, callowance,
			canChat, canPaint, sseq, aseq
		);

		if (world) {
			// TODO: move this into world
			SelfCursor& sc = world->getCursor();
			bool updated = sc.update(x, y, step, tid, tstate, actionBkt, chatBkt, sseq, aseq);
			if (updated) {
				world->getRenderer().queueRerender();
				world->getRenderer().queueUiUpdate();
			}
			return;
		}

		preJoinSelfCursorData = std::make_unique<SelfCursor::Builder>();
		SelfCursor::Builder& cur = *preJoinSelfCursorData.get();
		cur.setUser(users.at(selfUid))
			.setId(cid)
			.setSpawnX(x)
			.setSpawnY(y)
			.setStep(step)
			.setToolId(tid)
			.setToolState(tstate)
			.setActionBucket(actionBkt)
			.setChatBucket(chatBkt)
			.setCanChat(canChat)
			.setCanPaint(canPaint)
			.setStateSeq(sseq)
			.setActionSeq(aseq);
	});

	pr.on<CPlayersUpdt>([this](net::DAbsUpdAreaPos uaX, net::DAbsUpdAreaPos uaY,
			net::VPlayersHide hides, net::VPlayersShow shows, net::VPlayersUpdate updates) {
		world->handleUpdates(uaX, uaY, std::move(hides), std::move(shows), std::move(updates));
	});

	pr.on<CWorldData>(
		[this](std::string worldName, std::string motd, u32 bgClr, bool restricted, std::optional<User::Id> owner) {
			std::printf("WorldData: Name=%s BgClr=%X Restricted=%u Owner=", worldName.c_str(), bgClr, restricted);
			if (owner) {
				std::printf("%llX Motd=", *owner);
			} else {
				std::printf("(none) Motd=");
			}

			std::puts(motd.c_str());

			RGB_u bgClrU;
			bgClrU.rgb = bgClr;
			world = std::make_unique<World>(
				*this, im, std::move(worldName), std::move(preJoinSelfCursorData), bgClrU, restricted, std::move(owner)
			);

			set_loadscreen_visible(false);
			playAudioId("a-join");
		}
	);

	pr.on<CStats>([this](uvar worldCursors, uvar globalCursors) { // this is only received if we're in a world
		std::printf("Stats: CursorsInWorld=%llu CursorsInServer=%llu\n", worldCursors.get(), globalCursors.get());
		world->setCursorCount(worldCursors, globalCursors);
	});

	pr.on<CSubscribedAreas>([this](net::DAreaSyncSeq arseq, std::vector<std::tuple<net::DAbsWPos, net::DAbsWPos>> areasV) {
		std::vector<twoi32> areas;
		areas.reserve(areasV.size());
		for (auto [x, y] : areasV) {
			areas.emplace_back(mk_twoi32(x, y));
		}

		world->setSubscribedUpdateAreas(arseq, std::move(areas));
	});
}

void Client::tick() {
	im.tick();
	if (world) {
		world->tick();
	}
}

void Client::wsOpen() {
	setStatus("Connected!");
	std::puts("[Client] Ws opened");
}

void Client::wsClose(u16 code) {
	if (code != 4004) {
		setStatus("Disconnected!");
	} else {
		switch (lastError) {
		case CE_SESSION:
			setStatus("Not logged in!");
			set_login_prompt_visible(true);
			checkHttpSession([](bool hasSession) {
				if (!hasSession) {
					return;
				}

				set_login_prompt_visible(false);
				setStatus(R"(
						<p>You seem to be logged in, but the WebSocket server is not receiving the session cookie.</p>
						<p>
							<div>This can happen when blocking cookies globally.</div>
							<div>Please add a cookie usage exception to the wss:// or all protocol(s) of this page.</div>
						</p>
						<p>See <a href="https://crbug.com/947413" target="_blank">this Chromium bug</a> for more information.</p>
					)");
			});
			break;

		case CE_PROXY:
			setStatus("The server is not allowing proxy/VPN connections.");
			break;

		case CE_CAPTCHA:
			setStatus("Captcha token was invalid! Refresh and try again.");
			break;

		case CE_BAN:
			setStatus("You're banned!"); // todo: time remaining and reason
			break;

		case CE_WORLD:
			setStatus("Invalid world name! Allowed characters are a..z, 0..9, '_' and '.'");
			break;

		case CE_HEADER:
			setStatus("Can't connect from this domain, m8.");
			break;

		default:
			setStatus("weird error happened lol");
			break;
		}
	}

	set_loadscreen_visible(true);
	std::printf("[Client] Ws closed: %u\n", code);

	preJoinSelfCursorData = nullptr;
	world = nullptr;
	users.clear();
	selfUid = 0;
}

void Client::wsMessage(const char* buf, sz_t s, bool) {
	if (!pr.read(reinterpret_cast<const u8*>(buf), s)) {
		std::fprintf(stderr, "[Client] Unknown message received, opcode: %u\n", buf[0]);
	}
}

void Client::doWsOpen(void* d) {
	static_cast<Client*>(d)->wsOpen();
}

void Client::doWsClose(void* d, u16 code) {
	if (d) { // ws closes after Client gets destroyed
		static_cast<Client*>(d)->wsClose(code);
	}
}

void Client::doWsMessage(void* d, char* buf, sz_t s, bool txt) {
	static_cast<Client*>(d)->wsMessage(buf, s, txt);
}

World* Client::getWorld() {
	return world.get();
}

User& Client::getUser(User::Id uid) {
	auto [it, inserted] = users.try_emplace(uid, uid);

	if (inserted) {
		// load the user
	}

	return it->second;
}

void Client::doTick(void* d) {
	static_cast<Client*>(d)->tick();
}
