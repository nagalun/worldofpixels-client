#pragma once
#include <coroutine>
#include <cstddef>
#include <memory>
#include <utility>

extern "C" { // C++ -> JS functions
void cancel_async_request(int hdl);
}

int async_request(
	const char* url, const char* requesttype, const char* param, void* arg, int free,
	void (*onload)(unsigned, void*, void*, unsigned), void (*onerror)(unsigned, void*, int, const char*),
	void (*onprogress)(unsigned, void*, int, int)
);

struct awaitable_request {
	struct result {
		std::unique_ptr<char[], void (*)(void*)> data;
		std::size_t data_len;
		int err_http_code;
	};

	const char* url;
	const char* requesttype;
	const char* param;
	std::coroutine_handle<> co_h;
	std::unique_ptr<char[], void (*)(void*)> data;
	std::size_t data_len;
	int err_http_code;
	int request_hdl;

	awaitable_request(const char* url, const char* requesttype, const char* param);

	bool await_ready();
	void await_suspend(std::coroutine_handle<> h);
	result await_resume();
};

awaitable_request async_request(const char* url, const char* requesttype = "GET", const char* param = nullptr);
