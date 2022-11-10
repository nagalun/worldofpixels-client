#pragma once

#include "util/NonCopyable.hpp"
#include <cstddef>
#include <coroutine>
#include <utility>

struct suspend_maybe {
	bool should_suspend;
	bool await_ready() const noexcept;
	void await_suspend(std::coroutine_handle<> h) const noexcept;
	void await_resume() const noexcept;
};

template<typename RetType = void>
struct Async;

template<typename RetType>
struct BasePromise {
	std::coroutine_handle<> awaiter_h; // the handle of the function awaiting this task
	bool awaiter_destroyed = false;

	Async<RetType> get_return_object();
	std::suspend_never initial_suspend() noexcept;
	suspend_maybe final_suspend() noexcept;
	void unhandled_exception();
};

template<typename RetType>
struct Promise : BasePromise<RetType> {
	alignas(RetType) std::byte ret_buf[sizeof(RetType)];

	~Promise();
	RetType get_ret_val();
	void return_value(RetType);
};

template<>
struct Promise<void> : BasePromise<void> {
	void return_void() noexcept;
};

template<typename RetType>
struct Async : NonCopyable {
	using promise_type = Promise<RetType>;

	std::coroutine_handle<promise_type> task_h;

	Async(std::coroutine_handle<promise_type> p);
	~Async();
	Async(Async&& o) noexcept;

	bool await_ready() const noexcept;
	void await_suspend(std::coroutine_handle<> h);
	RetType await_resume();
};

#include "async.tpp" // IWYU pragma: keep
