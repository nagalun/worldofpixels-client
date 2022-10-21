#pragma once

#include <type_traits>
#include <utility>

template <typename T, std::size_t dataSize = sizeof(void*)> class SmallLambda;
template <typename R, typename... Args, std::size_t dataSize>
class SmallLambda<R(Args...), dataSize> {
	using Data = char[dataSize];
	using FnPtr = R(*)(Data*, Args...);
	FnPtr fn;
	Data data;

public:
	template<typename T>
	SmallLambda(T&& callable) { // also accepts nullptr
		*this = std::forward<T>(callable);
	}

	template<typename T>
	void operator=(T&& callable) {
		static_assert(sizeof(T) <= sizeof(data), "Object too big");
		static_assert(std::is_trivially_copyable_v<T>, "Functor is not trivially copyable");
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

