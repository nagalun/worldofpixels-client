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
	PipetteTool(std::tuple<ToolManager&, InputAdapter&>); // local ctor
	PipetteTool(ToolManager&); // remote ctor
	virtual ~PipetteTool();

	static const char * getNameSt();
	std::string_view getName() const override;
	bool isEnabled() override;

	uint8_t getNetId() const override;
//	const std::vector<std::uint8_t>& getNetState() const override;
//	void setStateFromNet(std::vector<std::uint8_t>) override;

	void onSelectionChanged(bool selected) override;
};

