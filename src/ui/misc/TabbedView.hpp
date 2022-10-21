#pragma once

#include <cstdint>
#include <type_traits>
#include <tuple>
#include "util/emsc/ui/Object.hpp"

namespace detail {
// "tgrp-2147483647"
constexpr std::size_t tgBufSz = 15;

template<typename Content>
struct Tab : public eui::Object {
	static_assert(std::is_base_of_v<eui::Object, Content>, "Tab content must be an element");

	eui::Object radio;
	eui::Object label;
	Content content;

	template<typename... CArgs>
	Tab(std::uint32_t tabGroupId, CArgs&&... cargs);
};

}

template<typename... TabContents>
class TabbedView : public eui::Object {
	std::tuple<detail::Tab<TabContents>...> tabs;

public:
	template<typename... CArgs>
	TabbedView(CArgs&&...);
};

#include "TabbedView.tpp" // IWYU pragma: keep
