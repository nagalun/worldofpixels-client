#pragma once

#include <cstdint>

class Client;
class World;
class Renderer;
class Camera;

class JsApiProxy {
	Client * cli;

	JsApiProxy();

public:
	JsApiProxy(const JsApiProxy&) = delete;
	void operator=(const JsApiProxy&) = delete;

	static JsApiProxy& getInstance();

	void setClientInstance(Client *);
	static Client * getClient();
	static World * getWorld();
	static Renderer * getRenderer();
	static Camera * getCamera();
};

