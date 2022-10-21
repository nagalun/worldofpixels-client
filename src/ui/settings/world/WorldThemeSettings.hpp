#pragma once

#include <cstdint>
#include <string_view>

#include "util/emsc/ui/Object.hpp"

class WorldThemeSettings: public eui::Object {
public:
	WorldThemeSettings();

	std::string_view getTabName() const;
};

