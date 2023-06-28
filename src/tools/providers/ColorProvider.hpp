#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include "util/color.hpp"
#include "util/NonCopyable.hpp"

class ToolManager;
class InputAdapter;

class ColorProvider : NonCopyable {
	struct LocalContext;
	std::unique_ptr<LocalContext> lctx;

public:
	class State {
		RGB_u primaryColor;
		RGB_u secondaryColor;

	public:
		State();

		RGB_u getPrimaryColor() const;
		RGB_u getSecondaryColor() const;

		bool swapColors();
		bool setPrimaryColor(RGB_u);
		bool setSecondaryColor(RGB_u);
	};

	ColorProvider(std::tuple<ToolManager&, InputAdapter&> params);
	~ColorProvider();
};
