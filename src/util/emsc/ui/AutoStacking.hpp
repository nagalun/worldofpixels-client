#pragma once

#include <util/emsc/ui/Object.hpp>

namespace eui {

class AutoStacking : public eui::Object {
	std::string_view containerSelector;

public:
	AutoStacking(std::string_view container);

	void bringUp();
};

}
