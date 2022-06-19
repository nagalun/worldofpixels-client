template<typename Fn>
constexpr void ToolManager::forEachTool(Fn cb) {
	std::apply([cb{std::move(cb)}] (auto&... x) {
		int i = 0;
		(cb(i++, x), ...);
	}, tools);
}

template<typename T>
constexpr T ToolManager::getTool() {
	return std::get<T>(tools);
}

template<typename T>
void ToolManager::selectTool() {
	selectTool(&std::get<T>(tools));
}
