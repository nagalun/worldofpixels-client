#pragma once

#include <cstdint>
#include <string_view>

#include <util/emsc/ui/Object.hpp>

class WorldAudioSettings: public eui::Object {
public:
	WorldAudioSettings();

	std::string_view getTabName() const;
};

