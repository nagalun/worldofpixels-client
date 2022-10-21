#include "audio.hpp"
#include <emscripten.h>

static bool audioEnabled = true;

EM_JS(bool, js_play_audio_id, (const char * buf, std::size_t len), {
	var a = document.getElementById(UTF8ToString(buf, len));
	if (a && a.play && a.volume > 0) {
		a.currentTime = 0;
		var p = a.play();
		if (p) {
			p.catch(function() {});
		}
		return true;
	}

	return false;
});

EM_JS(bool, js_set_volume_audio_id, (const char * buf, std::size_t len, float vol), {
	var a = document.getElementById(UTF8ToString(buf, len));
	if (a && a.play) {
		a.volume = vol;
		return true;
	}

	return false;
});

bool playAudioId(std::string_view s) {
	return audioEnabled && js_play_audio_id(s.data(), s.size());
}

bool setVolumeAudioId(std::string_view s, float vol) {
	return js_set_volume_audio_id(s.data(), s.size(), vol);
}

void setAudioEnabled(bool state) {
	audioEnabled = state;
}

bool getAudioEnabled() {
	return audioEnabled;
}
