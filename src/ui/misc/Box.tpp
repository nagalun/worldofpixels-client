#include <util/misc.hpp>
#include <utility>

template<typename Tuple, std::size_t... I>
void doTupleAppendTo(Tuple& t, eui::Object& o, std::index_sequence<I...>) {
	(std::get<I>(t).appendTo(o), ...);
}

template<typename Tuple>
void tupleAppendTo(Tuple& t, eui::Object& o) {
	doTupleAppendTo(t, o, std::make_index_sequence<std::tuple_size_v<Tuple>>());
}

template<typename... Content>
template<typename... CArgs>
Box<Content...>::Box(CArgs&&... contentArgs)
: content(cartesian_make_tuple<decltype(content)>(std::forward<CArgs>(contentArgs)...)) {

	addClass("box");
	tupleAppendTo(content, *this);
}

template<typename... Content>
template<typename T>
T& Box<Content...>::get() {
	return std::get<T>(content);
}

template<typename... Content>
template<typename T>
const T& Box<Content...>::get() const {
	return std::get<T>(content);
}

template<typename... Content>
template<std::size_t I>
const auto& Box<Content...>::get() const {
	return std::get<I>(content);
}
