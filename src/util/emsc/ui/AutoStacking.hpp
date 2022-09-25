#pragma once

#include <util/emsc/ui/Object.hpp>
#include <util/emsc/ui/EventHandle.hpp>

namespace eui {

class AutoStacking : public eui::Object {
	EventHandle eh;

public:
	AutoStacking();

	bool bringUp(bool force = false);
	void setClickBringUpEnabled(bool);
};

}
