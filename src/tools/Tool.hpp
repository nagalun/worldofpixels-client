#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "util/NonCopyable.hpp"

class ToolManager;

class Tool : NonCopyable {
protected:
	ToolManager& tm;

public:
	Tool(ToolManager&);

	virtual ~Tool();

	virtual std::string_view getName() const = 0;
	virtual std::uint8_t getToolVisualState() const;
	virtual bool isEnabled() = 0;

	// note: an empty network state must be valid, server may not send state in some circumstances
	virtual std::uint8_t getNetId() const = 0;
	virtual const std::vector<std::uint8_t>& getNetState() const;
	virtual void setStateFromNet(std::vector<std::uint8_t>);

	virtual void onSelectionChanged(bool selected) = 0;

	bool operator==(const Tool&) const;
};
