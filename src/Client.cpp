#include "Client.hpp"

#include <cstdio>

#include "jswebsockets.hpp"

#include "PacketDefinitions.hpp"

static const char testpkt[4] = {0, 0, 0, 0};

Client::Client() {
	js_ws_on_open(Client::doWsOpen);
	js_ws_on_close(Client::doWsClose);
	js_ws_on_message(Client::doWsMessage);
	js_ws_set_user_data(this);

	//wsMessage(&testpkt[0], 4, false);
	registerPacketTypes();
	//wsMessage(&testpkt[0], 4, false);
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
	pr.on<AuthProgress>([this] (std::string currentProcessor) {
		std::fputs("AuthProgress: ", stdout);
		std::puts(currentProcessor.c_str());
	});
}

void Client::wsOpen() {
	std::puts("Ws opened");
	//wsMessage(&testpkt[0], 4, false);
}

void Client::wsClose(u16 code) {
	std::string cstr(std::to_string(code));
	std::fputs("Ws closed: ", stdout);
	std::puts(cstr.c_str());
	//wsMessage(&testpkt[0], 4, false);
}

void Client::wsMessage(const char * buf, sz_t s, bool) {
	std::puts("Ws msg");
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
