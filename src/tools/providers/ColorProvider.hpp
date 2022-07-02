#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include <util/color.hpp>

class ToolManager;
class InputAdapter;

class ColorProvider {
	struct LocalContext;
	std::unique_ptr<LocalContext> lctx;

	RGB_u primaryColor;
	RGB_u secondaryColor;

public:
	ColorProvider(std::tuple<ToolManager&, InputAdapter&>); // local ctor
	ColorProvider(ToolManager&); // remote ctor
	~ColorProvider();

	RGB_u getPrimaryColor() const;
	RGB_u getSecondaryColor() const;

	void swapColors();
	void setPrimaryColor(RGB_u);
	void setSecondaryColor(RGB_u);
};

