#pragma once

#include <type_traits>
#include <utility>

template <typename T, std::size_t DataSize = sizeof(void*), std::size_t Align = alignof(void*)> class SmallLambda;
template <typename R, typename... Args, std::size_t DataSize, std::size_t Align>
class SmallLambda<R(Args...), DataSize, Align> {
	using Data = char[DataSize];
	using FnPtr = R(*)(Data*, Args...);
	FnPtr fn;
	alignas(Align) Data data;

public:
	template<typename T>
	SmallLambda(T&& callable) { // also accepts nullptr
		*this = std::forward<T>(callable);
	}

	template<typename T>
	void operator=(T&& callable) {
		static_assert(sizeof(T) <= sizeof(data), "Object too big");
		static_assert(std::is_trivially_copyable_v<T>, "Functor is not trivially copyable");
		static_assert(Align % alignof(T) == 0, "Improper storage alignment for functor");
		fn = &SmallLambda::call<T>;
		new (reinterpret_cast<T*>(&data[0])) T(std::forward<T>(callable));
	}

	void operator=(std::nullptr_t) {
		fn = nullptr;
	}

	inline R operator()(Args... args) {
		return fn(&data, std::forward<Args>(args)...);
	}

	operator bool() const {
		return fn;
	}

private:
	template<typename T>
	static R call(Data* data, Args... args) {
		T& t = (*reinterpret_cast<T*>(&(*data)[0]));
		return t(std::forward<Args>(args)...);
	}
};
