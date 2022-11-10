#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include "util/NonCopyable.hpp"
#include "util/explints.hpp"
#include "util/net/PacketReader.hpp"
#include "uvias/User.hpp"
#include "world/SelfCursor.hpp"

#include "InputManager.hpp"
#include "Settings.hpp"

enum EConnectError { CE_NONE, CE_PROXY, CE_CAPTCHA, CE_BAN, CE_SESSION, CE_WORLD, CE_HEADER };

class JsApiProxy;
class World;

class Client : NonCopyable {
public:
	static constexpr double ticksPerSec = 20.0;

private:
	JsApiProxy& api;
	InputManager im;
	InputAdapter& aClient;
	PacketReader pr;
	std::unordered_map<User::Id, User> users;
	std::unique_ptr<World> world;
	std::unique_ptr<SelfCursor::Builder> preJoinSelfCursorData;
#if __has_feature(address_sanitizer)
	ImAction iDoLeakCheck;
#endif
	User::Id selfUid;
	long tickTimer;
	EConnectError lastError;

	decltype(Settings::enableAudio)::SlotKey skAudioEnableCh;
	decltype(Settings::joinSfxVol)::SlotKey skJoinVolCh;
	decltype(Settings::buttonSfxVol)::SlotKey skButtonVolCh;
	decltype(Settings::paintSfxVol)::SlotKey skPaintVolCh;

public:
	Client(JsApiProxy&);
	~Client();

	bool open(std::string wsUrl, std::string_view worldToJoin);
	bool reconnect();
	void close();

	World* getWorld();

	bool freeMemory();

	static void setStatus(std::string_view);

private:
	void registerPacketTypes();

	void tick();

	void wsOpen();
	void wsClose(u16);
	void wsMessage(const char*, sz_t, bool);

	static void doWsOpen(void*);
	static void doWsClose(void*, u16);
	static void doWsMessage(void*, char*, sz_t, bool);

	static void doTick(void*);
};
