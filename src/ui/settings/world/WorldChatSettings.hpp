#pragma once

#include <cstdint>
#include <string_view>

#include "util/emsc/ui/Object.hpp"

class WorldChatSettings: public eui::Object {
public:
	WorldChatSettings();

	std::string_view getTabName() const;
};

