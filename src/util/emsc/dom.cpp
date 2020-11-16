#include <cstdio>
#include <emscripten.h>
#include "dom.hpp"

EM_JS(std::uint32_t, eui_create_elem, (const char * tag, std::size_t len), {
	if (!Module.EUI) {
		Module.EUI = {
			elemid: 0,
			elems: {}
		};
	}

	var id = ++Module.EUI.elemid;
	var e = document.createElement(UTF8ToString(tag, len));
	e.id = "eui-" + id;
	Module.EUI.elems[id] = e;

	return id;
});

EM_JS(void, eui_elem_add_class, (std::uint32_t id, const char * name, std::size_t len), {
	var e = Module.EUI.elems[id];
	e.classList.add(UTF8ToString(name, len));
});

EM_JS(void, eui_elem_del_class, (std::uint32_t id, const char * name, std::size_t len), {
	var e = Module.EUI.elems[id];
	e.classList.remove(UTF8ToString(name, len));
});

EM_JS(void, eui_append_elem, (std::uint32_t parentid, std::uint32_t id), {
	var e = Module.EUI.elems[parentid];
	var e2 = Module.EUI.elems[id];
	e.appendChild(e2);
});

EM_JS(void, eui_append_elem_selector, (const char * parent, std::size_t len, std::uint32_t id), {
	var e = document.querySelector(UTF8ToString(parent, len));
	var e2 = Module.EUI.elems[id];
	e.appendChild(e2);
});

EM_JS(void, eui_remove_elem, (std::uint32_t id), {
	var e = Module.EUI.elems[id];
	e.remove();
});

EM_JS(void, eui_destroy_elem, (std::uint32_t id), {
	var e = Module.EUI.elems[id];
	delete Module.EUI.elems[id];
	e.remove();
});

const char * eui_elem_selector(std::uint32_t id) {
	// "#eui-" + "4294967295" + '\0'
	static char buf[5 + 10 + 1] = "#eui-";
	std::sprintf(buf + 5, "%u", id);
	return buf;
}

EM_JS(std::size_t, eui_elem_property_len, (std::uint32_t id, const char * prop, std::size_t len), {
	var e = Module.EUI.elems[id];
	var prop = UTF8ToString(prop, len).split(".");
	var obj = e;
	while (prop.length > 1) obj = obj[prop.shift()];
	return prop[0] in obj ? obj[prop[0]].toString().length : 0;
});

EM_JS(std::size_t, eui_elem_property_get, (std::uint32_t id, const char * buf, std::size_t maxlen, const char * prop, std::size_t len), {
	var e = Module.EUI.elems[id];
	var prop = UTF8ToString(prop, len).split(".");
	var obj = e;
	while (prop.length > 1) obj = obj[prop.shift()];
	var val = prop[0] in obj ? obj[prop[0]].toString() : "";
	stringToUTF8(val, buf, maxlen);
	var written = lengthBytesUTF8(val); // stringToUTF8 could return the bytes written...
	return written >= maxlen ? maxlen : written;
});

EM_JS(std::size_t, eui_elem_property_set, (std::uint32_t id, const char * prop, std::size_t proplen, const char * val, std::size_t vallen), {
	var e = Module.EUI.elems[id];
	var prop = UTF8ToString(prop, proplen).split(".");
	var obj = e;
	while (prop.length > 1) obj = obj[prop.shift()];
	obj[prop[0]] = UTF8ToString(val, vallen);
});
