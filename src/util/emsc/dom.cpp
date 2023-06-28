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

EM_JS(bool, eui_elem_tgl_class, (std::uint32_t id, const char * name, std::size_t len), {
	var e = Module.EUI.elems[id];
	return e.classList.toggle(UTF8ToString(name, len));
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

EM_JS(void, eui_elem_add_handler, (std::uint32_t id, const char * evt, std::size_t len, void * data, EvtCb cb, bool passive), {
	var e = id !== 0 ? Module.EUI.elems[id] : window;
	var evts = Module.EUI.evts[id] = (Module.EUI.evts[id] || []);
	var evtNames = UTF8ToString(evt, len).split(" ");
	for (var i = 0; i < evtNames.length; i++) {
		var obj = {
			data: data,
			evt: evtNames[i],
			cb: cb,
			jscb: null,
			enabled: true
		};

		obj.jscb = function(ev) {
			if (!obj.enabled) { return; }
			Module.EUI.lastEvent = ev;
			if (Module["_eui_call_evt_handler"](obj.data, obj.cb)) {
				ev.preventDefault();
			}
		};

		e.addEventListener(obj.evt, obj.jscb, {"passive": !!passive});
		evts.push(obj);
	}
});

EM_JS(void, eui_elem_enable_handler, (std::uint32_t id, void * data, EvtCb cb, bool enabled), {
	var evts = Module.EUI.evts[id] = (Module.EUI.evts[id] || []);
	for (var i = 0; i < evts.length; i++) {
		var ev = evts[i];
		if (ev.data === data && ev.cb === cb) {
			ev.enabled = !!enabled;
		}
	}
});

EM_JS(void, eui_elem_ch_handler, (std::uint32_t id, void * data, EvtCb cb, void * newData, EvtCb newCb), {
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
	var e = id !== 0 ? Module.EUI.elems[id] : window;
	var evts = Module.EUI.evts[id];
	for (var i = 0; evts && i < evts.length; i++) {
		var ev = evts[i];
		if (ev.data === data && ev.cb === cb) {
			evts.splice(i, 1);
			--i;

			e.removeEventListener(ev.evt, ev.jscb);
		}
	}

	if (evts && !evts.length) {
		delete Module.EUI.evts[id];
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

EM_JS(std::size_t, eui_elem_property_get, (std::uint32_t id, char * buf, std::size_t maxlen, const char * prop, std::size_t len), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, len).split(".");
	var obj = e;
	while (sprop.length > 1) obj = obj[sprop.shift()];
	var val = sprop[0] in obj ? obj[sprop[0]].toString() : "";
	stringToUTF8(val, buf, maxlen);
	var written = lengthBytesUTF8(val); // stringToUTF8 could return the bytes written...
	return written >= maxlen ? maxlen : written;
});

EM_JS(bool, eui_elem_property_get_bool, (std::uint32_t id, const char * prop, std::size_t len), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, len).split(".");
	var obj = e;
	while (sprop.length > 1) obj = obj[sprop.shift()];
	return !!obj[sprop[0]];
});

EM_JS(void, eui_elem_property_set, (std::uint32_t id, const char * prop, std::size_t proplen, const char * val, std::size_t vallen), {
	var e = Module.EUI.elems[id];
	var sprop = UTF8ToString(prop, proplen).split(".");
	if (sprop.length == 2 && sprop[0] === "style" && sprop[1][0] === "-") {
		e.style.setProperty(sprop[1], UTF8ToString(val, vallen));
		return;
	}
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

EM_JS(std::size_t, eui_elem_attr_get, (std::uint32_t id, char * buf, std::size_t maxlen, const char * prop, std::size_t len), {
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

EM_JS(void, eui_elem_attr_del, (std::uint32_t id, const char * prop, std::size_t proplen), {
	var e = Module.EUI.elems[id];
	e.removeAttribute(UTF8ToString(prop, proplen));
});

EM_JS(void, eui_root_css_property_set, (const char * prop, std::size_t proplen, const char * val, std::size_t vallen), {
	var r = document.querySelector(":root");
	r.style.setProperty(UTF8ToString(prop, proplen), UTF8ToString(val, vallen));
});

EM_JS(void, eui_root_attr_set, (const char * prop, std::size_t proplen, const char * val, std::size_t vallen), {
	var r = document.querySelector(":root");
	r.setAttribute(UTF8ToString(prop, proplen), UTF8ToString(val, vallen));
});

EM_JS(void, eui_elem_ss_del_rule, (std::uint32_t id, std::size_t idx), {
	var e = Module.EUI.elems[id];
	e.sheet.deleteRule(idx);
});

EM_JS(void, eui_elem_ss_del_rules, (std::uint32_t id), {
	var e = Module.EUI.elems[id];
	if (e.sheet) {
		var n = e.sheet.cssRules.length;
		while (n) { e.sheet.deleteRule(--n); }
	}
});

EM_JS(void, eui_elem_ss_ins_rule, (std::uint32_t id, const char * rule, std::size_t rulelen, std::size_t idx), {
	var e = Module.EUI.elems[id];
	e.sheet.insertRule(UTF8ToString(rule, rulelen), idx);
});

EM_JS(void, eui_elem_ss_ins_rule_back, (std::uint32_t id, const char * rule, std::size_t rulelen), {
	var e = Module.EUI.elems[id];
	e.sheet.insertRule(UTF8ToString(rule, rulelen), e.sheet.cssRules.length);
});

EM_JS(void, eui_get_vp_size, (int * oww, int * owh), {
	HEAP32[oww / 4] = window.innerWidth;
	HEAP32[owh / 4] = window.innerHeight;
});

EM_JS(void, eui_get_elem_size, (std::uint32_t id, int * oew, int * oeh), {
	var e = Module.EUI.elems[id];
	HEAP32[oew / 4] = e.offsetWidth;
	HEAP32[oeh / 4] = e.offsetHeight;
});

EM_JS(void, eui_wait_n_frames, (int n, void * data, EvtCb cb), {
	function waitFrames(n, cb) {
		window.requestAnimationFrame(function() {
			return n > 1 ? waitFrames(--n, cb) : cb();
		});
	}

	waitFrames(n, function() {
		Module["_eui_call_evt_handler"](data, cb);
	});
});

// "blob:" (5) + origin + "/00000000-0000-0000-0000-000000000000" (37) + 1 nul
EM_JS(std::size_t, eui_blob_url_len, (void), {
	return location.origin.length + 43;
});

EM_JS(std::size_t, eui_blob_url_from_buf, (const std::uint8_t * buf, std::size_t len, const char * mime, std::size_t mimelen, char * urlbuf, std::size_t maxlen), {
	var b = new Blob([HEAPU8.subarray(buf, buf + len)], {"type": UTF8ToString(mime, mimelen)});
	var url = URL.createObjectURL(b);
	stringToUTF8(url, urlbuf, maxlen);
	var written = lengthBytesUTF8(url); // stringToUTF8 could return the bytes written...
	return written >= maxlen ? maxlen : written;
});

EM_JS(void, eui_blob_url_revoke, (const char * url, std::size_t len), {
	URL.revokeObjectURL(UTF8ToString(url, len));
});

EM_JS(void, eui_get_evt_pointer_coords, (int * x, int * y, bool clampWin, int * oww, int * owh), {
	var e = Module.EUI.lastEvent;
	var cx = 'clientX' in e ? e.clientX : e.touches[0].clientX;
	var cy = 'clientY' in e ? e.clientY : e.touches[0].clientY;
	var ww = window.innerWidth;
	var wh = window.innerHeight;
	if (clampWin) {
		cx = cx < 0 ? 0 : cx > ww ? ww : cx;
		cy = cy < 0 ? 0 : cy > wh ? wh : cy;
	}
	HEAP32[x / 4] = cx;
	HEAP32[y / 4] = cy;
	if (oww) { HEAP32[oww / 4] = ww; }
	if (owh) { HEAP32[owh / 4] = wh; }
});


EMSCRIPTEN_KEEPALIVE
bool eui_call_evt_handler(void * data, EvtCb cb) {
	return cb(data);
}

void eui_root_css_property_set(std::string_view prop, std::string_view val) {
	eui_root_css_property_set(prop.data(), prop.size(), val.data(), val.size());
}
