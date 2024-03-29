#include <emscripten.h>
#include "jswebsockets.hpp"

#include <cstdlib>
#include <cstring>

static char * msg_buf = nullptr;
static std::size_t msg_buf_sz = 0;

static void * usr_data = nullptr;

static void (*on_open)(void *) = nullptr;
static void (*on_close)(void *, std::uint16_t) = nullptr;
static void (*on_message)(void *, char *, std::size_t, bool) = nullptr;

EM_JS(bool, js_ws_open, (const char * url, std::size_t len, const char * proto, std::size_t protoLen), {
	"use strict";
	if (!Module.JSWS) {
		Module.JSWS = {
			ws: null,
			bufSize: 0,
			bufPtr: -1,
			lastProto: null
		};
	}

	if (Module.JSWS.ws && Module.JSWS.ws.readyState !== WebSocket.CLOSED) {
		return false;
	}

	try {
		var urlstr = UTF8ToString(url, len);
		var protostr = UTF8ToString(proto, protoLen);
		Module.JSWS.ws = new WebSocket(urlstr, protostr);
		Module.JSWS.lastProto = protostr;
	} catch (e) {
		console.log("js_ws_open:", e);
		return false;
	}

	Module.JSWS.ws.binaryType = "arraybuffer";
	Module.JSWS.ws.onopen = Module["_js_ws_call_on_open"];
	Module.JSWS.ws.onclose = function(e) { Module["_js_ws_call_on_close"](e.code); };
	Module.JSWS.ws.onmessage = function(e) {
		var data = e.data;
		if (data.byteLength > Module.JSWS.bufSize) {
			Module.JSWS.bufPtr = Module["_js_ws_prepare_msg_buffer"](data.byteLength);
			Module.JSWS.bufSize = HEAP32[Module.JSWS.bufPtr / 4];
		}

		HEAPU8.set(new Uint8Array(data), Module.JSWS.bufPtr);
		Module["_js_ws_call_on_message"](data.byteLength);
	};

	return true;
});

EM_JS(bool, js_ws_reconnect, (void), {
	if (Module.JSWS && Module.JSWS.ws && Module.JSWS.ws.readyState !== WebSocket.CLOSED) {
		return false;
	}

	var oldWs = Module.JSWS.ws;
	Module.JSWS.ws = new WebSocket(oldWs.url, Module.JSWS.lastProto);
	Module.JSWS.ws.binaryType = "arraybuffer";
	Module.JSWS.ws.onopen = oldWs.onopen;
	Module.JSWS.ws.onclose = oldWs.onclose;
	Module.JSWS.ws.onmessage = oldWs.onmessage;
	return true;
});

EM_JS(void, js_ws_close, (std::uint16_t code), {
	Module.JSWS.ws.close(code);
});

EM_JS(void, js_ws_send, (const char * buf, std::size_t len), {
	Module.JSWS.ws.send(HEAPU8.subarray(buf, buf + len));
});

EM_JS(void, js_ws_send_str, (const char * buf, std::size_t len), {
	Module.JSWS.ws.send(UTF8ToString(buf, len));
});

EM_JS(EWsReadyState, js_ws_get_ready_state, (void), {
	return Module.JSWS && Module.JSWS.ws ? Module.JSWS.ws.readyState : WebSocket.CLOSED;
});


void js_ws_set_user_data(void * usr) {
	usr_data = usr;
}

void js_ws_on_open(void (*cb)(void *)) {
	on_open = cb;
}

void js_ws_on_close(void (*cb)(void *, std::uint16_t)) {
	on_close = cb;
}

void js_ws_on_message(void (*cb)(void *, char *, std::size_t, bool)) {
	on_message = cb;
}


EMSCRIPTEN_KEEPALIVE
char * js_ws_prepare_msg_buffer(std::size_t sz) {
	if (sz < sizeof(std::size_t) || sz % sizeof(std::size_t)) {
		sz += sizeof(std::size_t) - (sz % sizeof(std::size_t));
	}

	if (sz > msg_buf_sz) {
		// use new std::size_t[] because we need the buffer to be aligned to it
		// also use over malloc to allow calling of OOM handler
		delete msg_buf;
		msg_buf = reinterpret_cast<char *>(new std::size_t[sz / sizeof(std::size_t)]);
		msg_buf_sz = sz;
	}

	std::memcpy(msg_buf, &msg_buf_sz, sizeof(std::size_t));
	return msg_buf;
}

EMSCRIPTEN_KEEPALIVE
void js_ws_call_on_open(void) {
	on_open(usr_data);
}

EMSCRIPTEN_KEEPALIVE
void js_ws_call_on_close(std::uint16_t code) {
	on_close(usr_data, code);
}

EMSCRIPTEN_KEEPALIVE
void js_ws_call_on_message(std::size_t sz) {
	on_message(usr_data, msg_buf, sz, false);
}

EMSCRIPTEN_KEEPALIVE
void js_ws_call_on_message_str(std::size_t sz) {
	on_message(usr_data, msg_buf, sz, true);
}
