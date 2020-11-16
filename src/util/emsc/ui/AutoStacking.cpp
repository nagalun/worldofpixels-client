#include <util/emsc/ui/AutoStacking.hpp>

#include <emscripten/html5.h>

using namespace eui;

AutoStacking::AutoStacking(std::string_view containerSelector)
: containerSelector(containerSelector) {
	appendTo(containerSelector);

	const char * target = getSelector().data();
	emscripten_set_mousedown_callback(target, this, true,
			+[] (int type, const EmscriptenMouseEvent *ev, void *data) -> int {
		AutoStacking * obj = static_cast<AutoStacking *>(data);

		obj->bringUp();

		return false;
	});
}

void AutoStacking::bringUp() {
	// appending an element again to the same parent re-orders it
	appendTo(containerSelector);
}
