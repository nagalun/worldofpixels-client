#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include "tools/Tool.hpp"

class ToolManager;
class InputAdapter;

class ZoomTool : public Tool {
	struct Keybinds;
	std::unique_ptr<Keybinds> kb;

public:
	class State {
		// negative zoom levels mean 1/-zoom
		std::int8_t zoom;

	public:
		State();

		std::int8_t getZoom() const;
		bool setZoom(std::int8_t z);
	};

	ZoomTool(std::tuple<ToolManager&, InputAdapter&>);
	virtual ~ZoomTool();

	static const char * getNameSt();
	std::string_view getName() const override;
	std::string_view getToolVisualName(const ToolStates&) const override;
	bool isEnabled() override;

	std::uint8_t getNetId() const override;
	std::uint64_t getNetState(const ToolStates& ts) const override;
	bool setStateFromNet(ToolStates& ts, std::uint64_t data) override;

	void onSelectionChanged(bool selected) override;
};
