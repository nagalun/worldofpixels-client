#include "util/emsc/ui/AutoStacking.hpp"

#include <cstdint>
#include <functional>

using namespace eui;

static std::uint32_t currentActiveId = 0;

AutoStacking::AutoStacking()
: eh(createHandler("click", [this] { return bringUp(); })) { }

AutoStacking::AutoStacking(AutoStacking&& o) noexcept
: eui::Object(std::move(o)),
  eh(std::move(o.eh)) {
	eh.setCb([this] { return bringUp(); });
}

const AutoStacking& AutoStacking::operator=(AutoStacking&& o) noexcept {
	eui::Object::operator =(std::move(o));
	eh = std::move(o.eh);
	eh.setCb([this] { return bringUp(); });
	return *this;
}

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
