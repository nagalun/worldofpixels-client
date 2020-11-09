#include "audio.hpp"

#include <emscripten.h>

EM_JS(bool, js_play_audio_id, (const char * buf, std::size_t len), {
	var a = document.getElementById(UTF8ToString(buf, len));
	if (a && a.play) {
		a.play();
		return true;
	}

	return false;
});

bool playAudioId(std::string_view s) {
	return js_play_audio_id(s.data(), s.size());
}