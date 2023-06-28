#include "time.hpp"

#include <emscripten.h>
#include <ratio>

double getTime(bool update) {
	static double ts = 0.0;

	if (update) {
		ts = emscripten_get_now() / 1000.0;
	}

	return ts;
}

std::chrono::steady_clock::time_point getStClock(bool update) {
	using namespace std::chrono;

	duration<double> ts{getTime(update)};
	steady_clock::time_point tp{duration_cast<steady_clock::duration>(ts)};
	return tp;
}
