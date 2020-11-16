#pragma once

#include <emscripten.h>

extern "C" { // C++ -> JS functions
	void cancel_async_request(int hdl);
}

int async_request(const char* url, const char* requesttype, const char* param, void *arg, int free, em_async_wget2_data_onload_func onload, em_async_wget2_data_onerror_func onerror, em_async_wget2_data_onprogress_func onprogress);
