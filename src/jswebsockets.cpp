#include "jswebsockets.hpp"

#include <emscripten.h>

#include <cstdlib>
#include <cstring>

static char * msg_buf = nullptr;
static std::size_t msg_buf_sz = 0;

static void * usr_data = nullptr;

static void (*on_open)(void *) = nullptr;
static void (*on_close)(void *, std::uint16_t) = nullptr;
static void (*on_message)(void *, char *, std::size_t, bool) = nullptr;

EM_JS(bool, js_ws_open, (const char * url, std::size_t len, const char * proto, std::size_t protoLen), {
	'use strict';
	if (!Module.JSWS) {
		Module.JSWS = {
			ws: null,
			bufSize: 0,
			bufPtr: -1
		};
	}

	if (Module.JSWS.ws && Module.JSWS.ws.readyState !== WebSocket.CLOSED) {
		return false;
	}

	try {
		var urlstr = Module.TXT.decode(url, len);
		var protostr = Module.TXT.decode(proto, protoLen);
		Module.JSWS.ws = new WebSocket(urlstr, protostr);
	} catch (e) {
		console.log('js_ws_open:', e);
		return false;
	}

	Module.JSWS.ws.binaryType = 'arraybuffer';
	Module.JSWS.ws.onopen = Module['_js_ws_call_on_open'];
	Module.JSWS.ws.onclose = function(e) { Module['_js_ws_call_on_close'](e.code); };
	Module.JSWS.ws.onmessage = function(e) {
		var data = e.data;
		if (data.byteLength > Module.JSWS.bufSize) {
			Module.JSWS.bufPtr = Module['_js_ws_prepare_msg_buffer'](data.byteLength);
			Module.JSWS.bufSize = Module['HEAP32'][Module.JSWS.bufPtr / Int32Array.BYTES_PER_ELEMENT];
		}

		Module['HEAPU8'].set(new Uint8Array(data), Module.JSWS.bufPtr);
		Module['_js_ws_call_on_message'](data.byteLength);
	};

	return true;
});

EM_JS(void, js_ws_close, (std::uint16_t code), {
	Module.JSWS.ws.close(code);
});

EM_JS(void, js_ws_send, (const char * buf, std::size_t len), {
	Module.JSWS.ws.send(Module['HEAPU8'].subarray(buf, buf + len));
});

EM_JS(void, js_ws_send_str, (const char * buf, std::size_t len), {
	Module.JSWS.ws.send(Module.TXT.decode(buf, len));
});

EM_JS(EWsReadyState, js_ws_get_ready_state, (void), {
	return Module.JSWS.ws ? Module.JSWS.ws.readyState : WebSocket.CLOSED;
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
	if (sz < sizeof(std::size_t)) {
		sz = sizeof(std::size_t);
	}

	if (sz > msg_buf_sz) {
		std::free(msg_buf);
		msg_buf = static_cast<char *>(std::malloc(sz));
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
