#pragma once

#include <cstdint>

class Client;

class JsApiProxy {
	Client * cli;

	JsApiProxy();

public:
	JsApiProxy(const JsApiProxy&) = delete;
	void operator=(const JsApiProxy&) = delete;

	static JsApiProxy& getInstance();

	void setClientInstance(Client *);
	Client * getClient();
};

