#pragma once

#include <cstdint>
#include <string_view>

#include <util/emsc/ui/Object.hpp>

class GeneralSettings: public eui::Object {
public:
	GeneralSettings();

	std::string_view getTabName() const;
};

