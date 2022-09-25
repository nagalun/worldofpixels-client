#pragma once

#include <cstdint>
#include <string_view>

#include <util/emsc/ui/Object.hpp>

class ThemeSettings: public eui::Object {
public:
	ThemeSettings();

	std::string_view getTabName() const;
};

