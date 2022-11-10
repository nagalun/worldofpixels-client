#pragma once
#include "async.hpp"

template<typename RetType>
Async<RetType> BasePromise<RetType>::get_return_object() {
	using T = typename Async<RetType>::promise_type;
	T* t = static_cast<T*>(this);
	return {std::coroutine_handle<T>::from_promise(*t)};
}

template<typename RetType>
std::suspend_never BasePromise<RetType>::initial_suspend() noexcept {
	return {};
}

template<typename RetType>
suspend_maybe BasePromise<RetType>::final_suspend() noexcept {
	if (awaiter_h) {
		awaiter_h.resume(); // .done() for the current task still returns false here, so it won't be destroyed yet
		return {false};     // if awaited end the coro, the promise object won't be used anymore. here it gets destroyed
	}

	// if the task ended without an awaiter it either means the task was synchronous or the obj didn't get awaited yet.
	// the promise will be destroyed later when it is awaited on, unless the awaiter is gone, in that case destroy now.
	return {!awaiter_destroyed};
}

template<typename RetType>
void BasePromise<RetType>::unhandled_exception() { }

template<typename RetType>
Promise<RetType>::~Promise() { // assumes a value is always returned before deletion
	reinterpret_cast<RetType*>(&ret_buf[0])->~RetType();
}

template<typename RetType>
RetType Promise<RetType>::get_ret_val(){
	return std::move(*reinterpret_cast<RetType*>(&ret_buf[0]));
}

template<typename RetType>
void Promise<RetType>::return_value(RetType val) {
	new (reinterpret_cast<RetType*>(&ret_buf[0])) RetType (std::move(val));
}

template<typename RetType>
Async<RetType>::Async(std::coroutine_handle<promise_type> p)
: task_h(std::move(p)) { }

template<typename RetType>
Async<RetType>::~Async() {
	if (task_h) {
		task_h.promise().awaiter_destroyed = true;
	}
}

template<typename RetType>
Async<RetType>::Async(Async&& o) noexcept
: task_h(std::exchange(o.task_h, nullptr)) { }

template<typename RetType>
RetType Async<RetType>::await_resume() {
	auto maybe_destroy = [this] {
		if (task_h && task_h.done()) { // .done() returns false if task was asynchronous
			task_h.destroy();
			task_h = nullptr;
		}
	};

	if constexpr (!std::is_same_v<RetType, void>) {
		auto val = task_h.promise().get_ret_val();
		maybe_destroy();
		return val;
	} else {
		maybe_destroy();
	}
}

template<typename RetType>
void Async<RetType>::await_suspend(std::coroutine_handle<> h) {
	task_h.promise().awaiter_h = std::move(h); // just supports one awaiter
}

template<typename RetType>
bool Async<RetType>::await_ready() const noexcept {
	bool done = !task_h || task_h.done();
	return done;
}
