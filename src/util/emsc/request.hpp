#pragma once

extern "C" { // C++ -> JS functions
	void cancel_async_request(int hdl);
}

int async_request(const char* url, const char* requesttype, const char* param, void *arg, int free, void (*onload)(unsigned, void*, void*, unsigned), void (*onerror)(unsigned, void*, int, const char*), void (*onprogress)(unsigned, void*, int, int));
