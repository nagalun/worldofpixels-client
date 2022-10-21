#pragma once

#include <cstdint>
#include <string_view>

#include "util/emsc/ui/Object.hpp"

class SocialSettings: public eui::Object {
public:
	SocialSettings();

	std::string_view getTabName() const;
};

