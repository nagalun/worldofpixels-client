#pragma once

#include <cstdint>
#include <string_view>

#include <util/emsc/ui/Object.hpp>

class WorldRolesSettings: public eui::Object {
public:
	WorldRolesSettings();

	std::string_view getTabName() const;
};

