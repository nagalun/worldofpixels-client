#pragma once

#include <cstdint>
#include <string_view>

#include <util/emsc/ui/Object.hpp>

class WorldSecuritySettings: public eui::Object {
public:
	WorldSecuritySettings();

	std::string_view getTabName() const;
};

