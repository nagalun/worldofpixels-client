#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include "tools/Tool.hpp"

class ToolManager;
class InputAdapter;

class PencilTool : public Tool {
	struct LocalContext;
	std::unique_ptr<LocalContext> lctx;

public:
	// max sent state size is 8 bytes. 4 bytes is used for the selected color which is set to the ColorProvider state
	class State {
		bool clicking;
		std::uint8_t brushSize;
		std::uint8_t brushShape;
		std::uint8_t ditherType;

	public:
		State();

		bool isClicking() const;
		std::uint8_t getBrushSize() const;
		std::uint8_t getBrushShape() const;
		std::uint8_t getDitherType() const;

		bool setClicking(bool s);
		bool setBrushSize(std::uint8_t s);
		bool setBrushShape(std::uint8_t s);
		bool setDitherType(std::uint8_t s);
	};

	PencilTool(std::tuple<ToolManager&, InputAdapter&>);
	virtual ~PencilTool();

	static const char * getNameSt();
	std::string_view getName() const override;
	std::string_view getToolVisualName(const ToolStates&) const override;
	bool isEnabled() override;

	std::uint8_t getNetId() const override;
	std::uint64_t getNetState(const ToolStates& ts) const override;
	bool setStateFromNet(ToolStates&, std::uint64_t) override;

	void onSelectionChanged(bool selected) override;
};
