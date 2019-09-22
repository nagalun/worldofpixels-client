#include "jswebsockets.hpp"

#include <cstdint>
#include <cstddef>
#include <cctype>

void ws_open(void *) {
	puts("opened");
}

void ws_close(void *, std::uint16_t code) {
	puts("closed");
}

void ws_msg(void *, char * buf, std::size_t sz, bool) {
	for (int i = 0; i < sz; i++) {
		if (!std::isprint(buf[i])) {
			buf[i] = '?';
		}
	}

	fwrite(buf, sizeof(char), sz, stdout);
	putchar('\n');
}

int main() {
	js_ws_on_open(ws_open);
	js_ws_on_close(ws_close);
	js_ws_on_message(ws_msg);

	js_ws_open("wss://dev.ourworldofpixels.com", 30, "OWOP", 4);
}
