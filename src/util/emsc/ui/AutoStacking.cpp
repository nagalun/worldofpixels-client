#include <util/emsc/ui/AutoStacking.hpp>

#include <cstdint>
#include <functional>

using namespace eui;

static std::uint32_t currentActiveId = 0;

AutoStacking::AutoStacking()
: eh(createHandler("click", [this] { return bringUp(false); })) { }

bool AutoStacking::bringUp(bool force) {
	// appending an element again to the same parent re-orders it
	if (currentActiveId != Object::getId() || force) {
		currentActiveId = Object::getId();
		appendToMainContainer();
	}

	return false;
}

void AutoStacking::setClickBringUpEnabled(bool state) {
	eh.setEnabled(state);
}
