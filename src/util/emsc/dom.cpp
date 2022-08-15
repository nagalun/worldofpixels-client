#include <cstdio>
#include <emscripten.h>
#include "dom.hpp"

using EvtCb = bool(*)(void *);

EM_JS(std::uint32_t, eui_create_elem, (const char * tag, std::size_t len), {
	if (!Module.EUI) {
		Module.EUI = {
			elemid: 0,
			elems: {},
			evts: {},
			lastEvent: null
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

EM_JS(void, eui_elem_add_handler, (std::uint32_t id, const char * evt, std::size_t len, void * data, EvtCb cb), {
	var e = Module.EUI.elems[id];
	var evts = Module.EUI.evts[id] = (Module.EUI.evts[id] || []);
	var evtNames = UTF8ToString(evt, len).split(" ");
	for (var i = 0; i < evtNames.length; i++) {
		var obj = {
			data: data,
			evt: evtNames[i],
			cb: cb,
			jscb: null
		};

		obj.jscb = function(ev) {
			Module.EUI.lastEvent = ev;
			if (Module["_eui_call_evt_handler"](obj.data, obj.cb)) {
				ev.preventDefault();
			}
		};

		e.addEventListener(obj.evt, obj.jscb);
		evts.push(obj);
	}
});

EM_JS(void, eui_elem_ch_handler, (std::uint32_t id, void * data, EvtCb cb, void * newData, EvtCb newCb), {
	var e = Module.EUI.elems[id];
	var evts = Module.EUI.evts[id] = (Module.EUI.evts[id] || []);
	for (var i = 0; i < evts.length; i++) {
		var ev = evts[i];
		if (ev.data === data && ev.cb === cb) {
			ev.cb = newCb;
			ev.data = newData;
		}
	}
});

EM_JS(void, eui_elem_del_handler, (std::uint32_t id, void * data, EvtCb cb), {
	var e = Module.EUI.elems[id];
	var evts = Module.EUI.evts[id] = (Module.EUI.evts[id] || []);
	for (var i = 0; i < evts.length; i++) {
		var ev = evts[i];
		if (ev.data === data && ev.cb === cb) {
			evts.splice(i, 1);
			--i;

			e.removeEventListener(ev.evt, ev.jscb);
		}
	}
});

const char * eui_elem_selector(std::uint32_t id) {
	// "#eui-" + "4294967295" + '\0'
	static char buf[5 + 10 + 1] = "#eui-";
	std::sprintf(buf + 5, "%u", id);
	return buf;
}

EM_JS(std::size_t, eui_elem_property_len, (std::uint32_t id, const char * prop, std::size_t len), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, len).split(".");
	var obj = e;
	while (sprop.length > 1) obj = obj[sprop.shift()];
	return sprop[0] in obj ? obj[sprop[0]].toString().length : 0;
});

EM_JS(std::size_t, eui_elem_property_get, (std::uint32_t id, const char * buf, std::size_t maxlen, const char * prop, std::size_t len), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, len).split(".");
	var obj = e;
	while (sprop.length > 1) obj = obj[sprop.shift()];
	var val = sprop[0] in obj ? obj[sprop[0]].toString() : "";
	stringToUTF8(val, buf, maxlen);
	var written = lengthBytesUTF8(val); // stringToUTF8 could return the bytes written...
	return written >= maxlen ? maxlen : written;
});

EM_JS(void, eui_elem_property_set, (std::uint32_t id, const char * prop, std::size_t proplen, const char * val, std::size_t vallen), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, proplen).split(".");
	var obj = e;
	while (sprop.length > 1) obj = obj[sprop.shift()];
	obj[sprop[0]] = UTF8ToString(val, vallen);
});

EM_JS(void, eui_elem_property_set_bool, (std::uint32_t id, const char * prop, std::size_t proplen, bool val), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, proplen).split(".");
	var obj = e;
	while (sprop.length > 1) obj = obj[sprop.shift()];
	obj[sprop[0]] = !!val;
});

EM_JS(std::size_t, eui_elem_attr_len, (std::uint32_t id, const char * prop, std::size_t len), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, len);
	return (e.getAttribute(sprop) || "").length;
});

EM_JS(std::size_t, eui_elem_attr_get, (std::uint32_t id, const char * buf, std::size_t maxlen, const char * prop, std::size_t len), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, len);
	var val = e.getAttribute(sprop) || "";
	stringToUTF8(val, buf, maxlen);
	var written = lengthBytesUTF8(val); // stringToUTF8 could return the bytes written...
	return written >= maxlen ? maxlen : written;
});

EM_JS(void, eui_elem_attr_set, (std::uint32_t id, const char * prop, std::size_t proplen, const char * val, std::size_t vallen), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, proplen);
	e.setAttribute(sprop, UTF8ToString(val, vallen));
});

EM_JS(void, eui_root_css_property_set, (const char * prop, std::size_t proplen, const char * val, std::size_t vallen), {
	var r = document.querySelector(":root");
	r.style.setProperty(UTF8ToString(prop, proplen), UTF8ToString(val, vallen));
});



EMSCRIPTEN_KEEPALIVE
bool eui_call_evt_handler(void * data, EvtCb cb) {
	return cb(data);
}

void eui_root_css_property_set(std::string_view prop, std::string_view val) {
	eui_root_css_property_set(prop.data(), prop.size(), val.data(), val.size());
}
