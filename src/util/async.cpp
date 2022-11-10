#include "async.hpp"

bool suspend_maybe::await_ready() const noexcept {
	return !should_suspend;
}

void suspend_maybe::await_suspend(std::coroutine_handle<>) const noexcept { }

void suspend_maybe::await_resume() const noexcept { }

void Promise<void>::return_void() noexcept { }
