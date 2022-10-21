#include "ui/settings/WorldSettings.hpp"

WorldSettings::WorldSettings() {
	tabs.appendTo(*this);

}

std::string_view WorldSettings::getTabName() const {
	return "World";
}
