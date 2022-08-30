#pragma once

#include <cstdint>
#include <cstddef>

enum EWsReadyState : std::uint8_t {
	CONNECTING,
	OPEN,
	CLOSING,
	CLOSED
};

extern "C" { // C++ -> JS functions
	bool js_ws_open(const char * url, std::size_t, const char * proto = nullptr, std::size_t = 0);
	bool js_ws_reconnect(void);
	void js_ws_close(std::uint16_t code);

	void js_ws_send(const char *, std::size_t); // sends as binary data
	void js_ws_send_str(const char *, std::size_t);

	EWsReadyState js_ws_get_ready_state(void);
}

void js_ws_set_user_data(void *);
void js_ws_on_open(void (*)(void *));
void js_ws_on_close(void (*)(void *, std::uint16_t));
void js_ws_on_message(void (*)(void *, char *, std::size_t, bool));

extern "C" { // JS -> C++ functions
	// returns actual buffer size in first index as int
	char * js_ws_prepare_msg_buffer(std::size_t);
	void js_ws_call_on_open(void);
	void js_ws_call_on_close(std::uint16_t);
	void js_ws_call_on_message(std::size_t); // buf ptr not needed
	void js_ws_call_on_message_str(std::size_t);
}
