#include "Client.hpp"

#include <cstdio>
#include <optional>

#include <emscripten.h>
#include <emscripten/html5.h>
#include <util/Bucket.hpp>
#include <util/emsc/audio.hpp>
#include <util/emsc/jswebsockets.hpp>

#include <PacketDefinitions.hpp>

// TODO: Translate strings

EM_JS(void, set_client_status, (const char * buf, std::size_t len), {
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

Client::Client()
: im(EMSCRIPTEN_EVENT_TARGET_WINDOW, "#world"),
  aClient(im.mkAdapter("Client", -1)),
  selfUid(0),
  globalCursorCount(0),
  tickTimer(emscripten_set_interval(Client::doTick, 1000.0 / Client::ticksPerSec, this)),
  lastError(CE_NONE) {
	std::printf("[Client] Created\n");
	js_ws_on_open(Client::doWsOpen);
	js_ws_on_close(Client::doWsClose);
	js_ws_on_message(Client::doWsMessage);
	js_ws_set_user_data(this);

	registerPacketTypes();
}

Client::~Client() {
	setStatus("Unloading...");

	if (js_ws_get_ready_state() != EWsReadyState::CLOSED) {
		js_ws_close(4001);
	}

	js_ws_set_user_data(nullptr); // prev ptr won't be valid when i return
	emscripten_clear_interval(tickTimer);
	std::puts("[Client] Destroyed");
}

bool Client::open(std::string_view worldToJoin) {
	setStatus("Connecting...");

	std::string url("wss://dev.ourworldofpixels.com/");
	url += worldToJoin;

	return js_ws_open(url.data(), url.size(), "OWOP", 4);
}

void Client::close() {
	js_ws_close(4000);
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
	pr.on<AuthProgress>([this] (std::string currentProcessor) {
		setStatus("Authenticating... (" + currentProcessor + ")");
		std::printf("AuthProgress: %s\n", currentProcessor.c_str());
	});

	pr.on<AuthOk>([this] (User::Id _selfUid, std::string username, User::Rep totRep,
			UviasRank::Id rid, std::string rankName, bool isSuperUser, bool canSelfManage) {
		setStatus("Joining world...");

		std::printf("AuthOk: Uid=%llX Username=%s TotalRep=%i RankId=%u RankName=%s SuperUser=%u CanSelfManage=%u\n",
			_selfUid, username.c_str(), totRep, rid, rankName.c_str(), isSuperUser, canSelfManage);

		selfUid = _selfUid;
		users.try_emplace(selfUid, selfUid, totRep, UviasRank(rid, std::move(rankName), isSuperUser, canSelfManage), std::move(username));
	});

	pr.on<AuthError>([this] (std::string processor) {
		setStatus("Auth error: " + processor);
		std::printf("AuthError: %s\n", processor.c_str());

		if (processor == "SessionChecker") {
			lastError = CE_SESSION;
		} else if (processor == "BanChecker") {
			lastError = CE_BAN;
		} else if (processor == "WorldChecker") {
			lastError = CE_WORLD;
		} else if (processor == "HeaderChecker") {
			lastError = CE_HEADER;
		} else if (processor == "ProxyChecker") {
			lastError = CE_PROXY;
		} else if (processor == "CaptchaChecker") {
			lastError = CE_CAPTCHA;
		} else {
			lastError = CE_NONE;
		}
	});

	pr.on<CursorData>([this] (net::Cursor selfCur, net::Bucket paint, net::Bucket chat, bool canChat, bool canPaint) {
		auto [cid, x, y, step, tid] = selfCur;
		auto [prate, pper, pallowance] = paint;
		auto [crate, cper, callowance] = chat;

		std::printf("CursorData: ID=%u X=%i Y=%i Step=%u ToolID=%u PBucketRate=%u PBucketPer=%u PBucketAllowance=%f CBucketRate=%u CBucketPer=%u CBucketAllowance=%f CanChat=%u CanPaint=%u\n",
				cid, x, y, step, tid, prate, pper, pallowance, crate, cper, callowance, canChat, canPaint);

		// cid, prate and crate has type due to eclipse cdt bug
		preJoinSelfCursorData = std::make_unique<SelfCursor>(
				users.at(selfUid), Cursor::Id{cid}, x, y, step, tid,
				Bucket(Bucket::Rate{prate}, pper, pallowance), Bucket(Bucket::Rate{crate}, cper, callowance),
				canChat, canPaint);
	});

	pr.on<WorldData>([this] (std::string worldName, std::string motd, u32 bgClr, bool restricted, std::optional<User::Id> owner) {
		std::printf("WorldData: Name=%s BgClr=%X Restricted=%u Owner=",
				worldName.c_str(), bgClr, restricted);
		if (owner) {
			std::printf("%llX Motd=", *owner);
		} else {
			std::printf("(none) Motd=");
		}

		std::puts(motd.c_str());

		RGB_u bgClrU;
		bgClrU.rgb = bgClr;
		world = std::make_unique<World>(im, std::move(worldName), std::move(preJoinSelfCursorData), bgClrU, restricted, std::move(owner));

		set_loadscreen_visible(false);
		playAudioId("a-join");
	});

	pr.on<Stats>([this] (u32 worldCursors, u32 globalCursors) { // this is only received if we're in a world
		std::printf("Stats: CursorsInWorld=%u CursorsInServer=%u\n", worldCursors, globalCursors);
		world->setCursorCount(worldCursors);
		globalCursorCount = globalCursors;
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
	globalCursorCount = 0;
}

void Client::wsMessage(const char * buf, sz_t s, bool) {
	if (!pr.read(reinterpret_cast<const u8 *>(buf), s)) {
		std::fprintf(stderr, "[Client] Unknown message received, opcode: %u\n", buf[0]);
	}
}


void Client::doWsOpen(void * d) {
	static_cast<Client *>(d)->wsOpen();
}

void Client::doWsClose(void * d, u16 code) {
	if (d) { // ws closes after Client gets destroyed
		static_cast<Client *>(d)->wsClose(code);
	}
}

void Client::doWsMessage(void * d, char * buf, sz_t s, bool txt) {
	static_cast<Client *>(d)->wsMessage(buf, s, txt);
}

void Client::doTick(void * d) {
	static_cast<Client *>(d)->tick();
}
