#include "Client.hpp"

#include <cstdio>
#include <optional>

#include <emscripten/html5.h>

#include "jswebsockets.hpp"

#include "PacketDefinitions.hpp"

Client::Client()
: im(EMSCRIPTEN_EVENT_TARGET_WINDOW),
  aClient(im.mkAdapter("Client", -1)),
  selfUid(0),
  globalCursorCount(0),
  tickTimer(emscripten_set_interval(Client::doTick, 1000.0 / Client::ticksPerSec, this)) {
	js_ws_on_open(Client::doWsOpen);
	js_ws_on_close(Client::doWsClose);
	js_ws_on_message(Client::doWsMessage);
	js_ws_set_user_data(this);

	registerPacketTypes();

	iTest = aClient.add("Print Test", {{"A"}}, T_ONPRESS | T_ONHOLD | T_ONRELEASE, [] (const auto&, auto t, const auto&) {
		switch (t) {
			case T_ONPRESS:
				std::printf("[Client] Test success: T_ONPRESS\n");
				break;

			case T_ONHOLD:
				std::printf("[Client] Test success: T_ONHOLD\n");
				break;

			case T_ONRELEASE:
				std::printf("[Client] Test success: T_ONRELEASE\n");
				break;
		}
	});
}

Client::~Client() {
	if (js_ws_get_ready_state() != EWsReadyState::CLOSED) {
		js_ws_close(4001);
	}

	js_ws_set_user_data(nullptr); // prev ptr won't be valid when i return
	emscripten_clear_interval(tickTimer);
	std::puts("[Client] Destroyed");
}

bool Client::open(std::string_view worldToJoin) {
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


void Client::registerPacketTypes() {
	pr.on<AuthProgress>([] (std::string currentProcessor) {
		std::printf("AuthProgress: %s\n", currentProcessor.c_str());
	});

	pr.on<AuthOk>([this] (User::Id _selfUid, std::string username, User::Rep totRep,
			UviasRank::Id rid, std::string rankName, bool isSuperUser, bool canSelfManage) {
		std::printf("AuthOk: Uid=%llX Username=%s TotalRep=%i RankId=%u RankName=%s SuperUser=%u CanSelfManage=%u\n",
			_selfUid, username.c_str(), totRep, rid, rankName.c_str(), isSuperUser, canSelfManage);

		selfUid = _selfUid;
		users.try_emplace(selfUid, selfUid, totRep, UviasRank(rid, std::move(rankName), isSuperUser, canSelfManage), std::move(username));
	});

	pr.on<AuthError>([] (std::string processor) {
		std::printf("AuthError: %s\n", processor.c_str());
	});

	pr.on<CursorData>([this] (net::Cursor selfCur, net::Bucket paint, net::Bucket chat, bool canChat, bool canPaint) {
		auto [cid, x, y, step, tid] = selfCur;
		auto [prate, pper, pallowance] = paint;
		auto [crate, cper, callowance] = chat;

		std::printf("CursorData: ID=%u X=%i Y=%i Step=%u ToolID=%u PBucketRate=%u PBucketPer=%u PBucketAllowance=%f CBucketRate=%u CBucketPer=%u CBucketAllowance=%f CanChat=%u CanPaint=%u\n",
				cid, x, y, step, tid, prate, pper, pallowance, crate, cper, callowance, canChat, canPaint);

		preJoinSelfCursorData = std::make_unique<SelfCursor>(users.at(selfUid), cid, x, y, step, tid, Bucket(prate, pper, pallowance), Bucket(crate, cper, callowance), canChat, canPaint);
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
	std::puts("[Client] Ws opened");
}

void Client::wsClose(u16 code) {
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
