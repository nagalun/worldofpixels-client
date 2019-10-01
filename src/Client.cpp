#include "Client.hpp"

#include <cstdio>
#include <optional>

#include "jswebsockets.hpp"

#include "PacketDefinitions.hpp"

Client::Client() {
	js_ws_on_open(Client::doWsOpen);
	js_ws_on_close(Client::doWsClose);
	js_ws_on_message(Client::doWsMessage);
	js_ws_set_user_data(this);

	registerPacketTypes();
}

bool Client::open(std::string_view worldToJoin) {
	std::string url("wss://dev.ourworldofpixels.com/");
	url += worldToJoin;

	return js_ws_open(url.data(), url.size(), "OWOP", 4);
}

void Client::close() {
	js_ws_close(4000);
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

	pr.on<CursorData>([] (net::Cursor selfCur, net::Bucket paint, net::Bucket chat, bool canChat, bool canPaint) {
		auto [cid, x, y, step, tid] = selfCur;
		auto [prate, pper, pallowance] = paint;
		auto [crate, cper, callowance] = chat;

		std::printf("CursorData: ID=%u X=%i Y=%i Step=%u ToolID=%u PBucketRate=%u PBucketPer=%u PBucketAllowance=%f CBucketRate=%u CBucketPer=%u CBucketAllowance=%f CanChat=%u CanPaint=%u\n",
				cid, x, y, step, tid, prate, pper, pallowance, crate, cper, callowance, canChat, canPaint);
	});

	pr.on<WorldData>([] (std::string worldName, std::string motd, u32 bgClr, bool restricted, std::optional<User::Id> owner) {
		std::printf("WorldData: Name=%s BgClr=%X Restricted=%u Owner=",
				worldName.c_str(), bgClr, restricted);
		if (owner) {
			std::printf("%llX Motd=", *owner);
		} else {
			std::printf("(none) Motd=");
		}

		std::puts(motd.c_str());
	});

	pr.on<Stats>([] (u32 worldCursors, u32 globalCursors) {
		std::printf("Stats: CursorsInWorld=%u CursorsInServer=%u\n", worldCursors, globalCursors);
	});
}

void Client::wsOpen() {
	std::puts("Ws opened");
}

void Client::wsClose(u16 code) {
	std::string cstr(std::to_string(code));
	std::fputs("Ws closed: ", stdout);
	std::puts(cstr.c_str());
}

void Client::wsMessage(const char * buf, sz_t s, bool) {
	if (!pr.read(reinterpret_cast<const u8 *>(buf), s)) {
		std::string opc(std::to_string(u16(buf[0])));
		std::fputs("Unknown message received, opcode: ", stderr);
		std::fputs(opc.c_str(), stderr);
		std::fputc('\n', stderr);
	}
}


void Client::doWsOpen(void * d) {
	static_cast<Client *>(d)->wsOpen();
}

void Client::doWsClose(void * d, u16 code) {
	static_cast<Client *>(d)->wsClose(code);
}

void Client::doWsMessage(void * d, char * buf, sz_t s, bool txt) {
	static_cast<Client *>(d)->wsMessage(buf, s, txt);
}
