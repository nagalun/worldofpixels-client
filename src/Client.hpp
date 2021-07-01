#pragma once

#include <util/explints.hpp>
#include <util/net/PacketReader.hpp>
#include <uvias/User.hpp>
#include <world/SelfCursor.hpp>
#include <world/World.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <string_view>

#include <InputManager.hpp>

enum EConnectError {
	CE_NONE,
	CE_PROXY,
	CE_CAPTCHA,
	CE_BAN,
	CE_SESSION,
	CE_WORLD,
	CE_HEADER
};

class JsApiProxy;

// UBSan complains that the class is not aligned to 16-bytes
// when constructing it with make_unique...
class alignas(32) Client {
public:
	static constexpr double ticksPerSec = 20.0;

private:
	JsApiProxy& api;
	InputManager im;
	InputAdapter& aClient;
	PacketReader pr;
	std::unordered_map<User::Id, User> users;
	std::unique_ptr<World> world;
	std::unique_ptr<SelfCursor> preJoinSelfCursorData;
	User::Id selfUid;
	u32 globalCursorCount;
	long tickTimer;
	EConnectError lastError;

public:
	Client(JsApiProxy&);
	~Client();

	bool open(std::string_view worldToJoin);
	void close();

	World * getWorld();

	bool freeMemory();

private:
	void setStatus(std::string_view);
	void registerPacketTypes();

	void tick();

	void wsOpen();
	void wsClose(u16);
	void wsMessage(const char *, sz_t, bool);

	static void doWsOpen(void *);
	static void doWsClose(void *, u16);
	static void doWsMessage(void *, char *, sz_t, bool);

	static void doTick(void *);
};
