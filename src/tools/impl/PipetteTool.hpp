#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include "tools/Tool.hpp"

class ToolManager;
class InputAdapter;

class PipetteTool : public Tool {
	struct Keybinds;
	std::unique_ptr<Keybinds> kb;

public:
	class State {
		bool clicking;

	public:
		State();

		bool isClicking() const;
		bool setClicking(bool st);
	};

	PipetteTool(std::tuple<ToolManager&, InputAdapter&>); // local ctor
	virtual ~PipetteTool();

	static const char * getNameSt();
	std::string_view getName() const override;
	std::string_view getToolVisualName(const ToolStates&) const override;
	bool isEnabled() override;

	std::uint8_t getNetId() const override;
	std::uint64_t getNetState(const ToolStates& ts) const override;
	bool setStateFromNet(ToolStates& ts, std::uint64_t data) override;

	void onSelectionChanged(bool selected) override;
};
