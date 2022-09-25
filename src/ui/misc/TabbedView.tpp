#include <util/misc.hpp>

template<typename Content>
template<typename... CArgs>
detail::Tab<Content>::Tab(std::uint32_t tabGroupId, CArgs&&... cargs)
: eui::Object("div"),
  radio("input"),
  label("label"),
  content(std::forward<CArgs>(cargs)...) {
	auto idSel = radio.getSelector();
	idSel.remove_prefix(1); // remove # from the beginning

	radio.setAttribute("type", "radio");
	radio.setAttribute("name", svprintf<tgBufSz>("tgrp-%d", tabGroupId));
	label.setAttribute("for", idSel);
	label.setAttribute("tabindex", "0"); // isn't helpful
	content.addClass("content");

	label.setProperty("textContent", content.getTabName());

	addClass("tab");
	radio.appendTo(*this);
	label.appendTo(*this);
	content.appendTo(*this);
}

template<typename... TabContents>
template<typename... CArgs>
TabbedView<TabContents...>::TabbedView(CArgs&&... contentArgs)
: eui::Object("div"),
  tabs(cartesian_make_tuple<decltype(tabs)>(getId(), std::forward<CArgs>(contentArgs)...)) {

	setProperty("style.--num-tabs", svprintf<detail::tgBufSz>("%d", std::tuple_size_v<decltype(tabs)>));
	addClass("tabs");

	if constexpr (std::tuple_size_v<decltype(tabs)> != 0) {
		std::get<0>(tabs).radio.setAttribute("checked");
	}

	(std::get<detail::Tab<TabContents>>(tabs).appendTo(*this), ...);
}
