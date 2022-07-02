#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include <tools/Tool.hpp>

class ToolManager;
class InputAdapter;

class PencilTool : public Tool {
	struct LocalContext;
	std::unique_ptr<LocalContext> lctx;

public:
	PencilTool(std::tuple<ToolManager&, InputAdapter&>); // local ctor
	PencilTool(ToolManager&); // remote ctor
	virtual ~PencilTool();

	static const char * getNameSt();
	std::string_view getName() const override;
	bool isEnabled() override;

	uint8_t getNetId() const override;
//	const std::vector<std::uint8_t>& getNetState() const override;
//	void setStateFromNet(std::vector<std::uint8_t>) override;

	void onSelectionChanged(bool selected) override;
};

