#include "request.hpp"

#include <string>

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
