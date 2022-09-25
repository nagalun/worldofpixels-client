
template<typename Fn>
UiButton::UiButton(std::string_view type, std::string_view title, bool themed, Fn cb)
: eui::Button(std::move(cb)) {
	if (themed) {
		addClass("eui-themed");
	}

	addClass("owop-ui");
	setAttribute("data-i", type);

	if (title.size() > 0) {
		setAttribute("title", title);
	}
}
