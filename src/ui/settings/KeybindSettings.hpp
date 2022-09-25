#pragma once

#include <cstdint>
#include <string_view>

#include <util/emsc/ui/Object.hpp>

class KeybindSettings: public eui::Object {
public:
	KeybindSettings();

	std::string_view getTabName() const;
};

