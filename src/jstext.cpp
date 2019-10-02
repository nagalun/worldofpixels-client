#include "jstext.hpp"

#include <emscripten.h>

EM_JS(void, js_txt_init, (void), {
	Module.TXT = {
		decode: function(buf, len) {
			return this.decode(Module['HEAPU8'].subarray(buf, buf + len));
		}.bind(new TextDecoder('utf-8'))
	};
});
