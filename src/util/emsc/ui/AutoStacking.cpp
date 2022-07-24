#include <util/emsc/ui/AutoStacking.hpp>

#include <cstdint>
#include <functional>

using namespace eui;

static std::uint32_t currentActiveId = 0;

AutoStacking::AutoStacking()
: eh(createHandler("click", std::bind(&AutoStacking::bringUp, this))){
	currentActiveId = Object::getId();
	appendToMainContainer();
}

bool AutoStacking::bringUp() {
	// appending an element again to the same parent re-orders it
	if (currentActiveId != Object::getId()) {
		currentActiveId = Object::getId();
		appendToMainContainer();
	}

	return false;
}
