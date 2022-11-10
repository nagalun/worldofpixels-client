#include "request.hpp"

#include <cstdlib>
#include <string>
#include <emscripten.h>

EM_JS(void, cancel_async_request, (int hdl), {
	var http = wget.wgetRequests[hdl];
	if (http) {
		http.onload = null;
		http.onerror = null;
		http.onprogress = null;
		http.onabort = null;
		http.abort();
		delete wget.wgetRequests[hdl];
	}
});

int async_request(const char* url, const char* requesttype, const char* param, void *arg, int free, em_async_wget2_data_onload_func onload, em_async_wget2_data_onerror_func onerror, em_async_wget2_data_onprogress_func onprogress) {
	const char * realurl = url;

#ifdef DEBUG_BASE_URL
	std::string base(DEBUG_BASE_URL);
	base += url;
	realurl = base.c_str();
#endif

	return emscripten_async_wget2_data(realurl, requesttype, param, arg, free, onload, onerror, onprogress);
}

bool awaitable_request::await_ready() {
	return request_hdl == -2;
}

void awaitable_request::await_suspend(std::coroutine_handle<> h) {
	auto ok = +[] (unsigned, void* d, void* buf, unsigned len) {
		awaitable_request& ar = *static_cast<awaitable_request*>(d);
		ar.data.reset(static_cast<char*>(buf));
		ar.data_len = len;
		ar.request_hdl = -2;
		ar.co_h.resume();
	};

	auto err = +[] (unsigned, void* d, int code, const char* err) {
		awaitable_request& ar = *static_cast<awaitable_request*>(d);
		ar.err_http_code = code;
		ar.request_hdl = -2;
		ar.co_h.resume();
	};

	co_h = std::move(h);
	request_hdl = async_request(url, requesttype, param, this, false, ok, err, nullptr);
}

awaitable_request::result awaitable_request::await_resume() {
	return {std::move(data), data_len, err_http_code};
}

awaitable_request async_request(const char* url, const char* requesttype, const char* param) {
	return {url, requesttype, param};
}

awaitable_request::awaitable_request(const char* url, const char* requesttype, const char* param)
: url(url),
  requesttype(requesttype),
  param(param),
  data(nullptr, free),
  data_len(0),
  err_http_code(0),
  request_hdl(-1) { }
