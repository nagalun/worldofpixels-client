#pragma once

#include <cstddef>
#include <functional>

#include <util/emsc/ui/Button.hpp>

class UiButton : public eui::Button {
public:
	template<typename Fn = std::nullptr_t>
	UiButton(std::string_view type, std::string_view title = "", bool themed = true, Fn cb = nullptr);
};

#include "UiButton.tpp"
