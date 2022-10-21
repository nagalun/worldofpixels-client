#pragma once

#include <util/emsc/ui/Object.hpp>
#include <util/emsc/ui/EventHandle.hpp>

namespace eui {

class AutoStacking : public eui::Object {
	EventHandle eh;

public:
	AutoStacking();
	AutoStacking(AutoStacking&&) noexcept;
	const AutoStacking& operator=(AutoStacking&&) noexcept;

	bool bringUp(bool force = false);
	void setClickBringUpEnabled(bool);
};

}
