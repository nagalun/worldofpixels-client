#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "PacketReader.hpp"
#include "explints.hpp"

//#include "World.hpp"

class Client {
	PacketReader pr;
	//std::unique_ptr<World> world;

public:
	Client();

	bool open(std::string_view worldToJoin);
	void close();

private:
	void registerPacketTypes();

	void wsOpen();
	void wsClose(u16);
	void wsMessage(const char *, sz_t, bool);

	static void doWsOpen(void *);
	static void doWsClose(void *, u16);
	static void doWsMessage(void *, char *, sz_t, bool);
};
