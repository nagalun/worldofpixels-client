#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

extern "C" { // C++ -> JS
	std::uint32_t eui_create_elem(const char * tag, std::size_t len);
	void eui_elem_add_class(std::uint32_t id, const char * name, std::size_t len);
	bool eui_elem_tgl_class(std::uint32_t id, const char * name, std::size_t len);
	void eui_elem_del_class(std::uint32_t id, const char * name, std::size_t len);
	void eui_append_elem(std::uint32_t parentid, std::uint32_t id);
	void eui_append_elem_selector(const char * parent, std::size_t len, std::uint32_t id);
	void eui_remove_elem(std::uint32_t id);
	void eui_destroy_elem(std::uint32_t id);
	void eui_elem_add_handler(std::uint32_t id, const char * evt, std::size_t len, void * data, bool(*cb)(void *), bool passive);
	void eui_elem_enable_handler(std::uint32_t id, void * data, bool(*cb)(void *), bool enabled);
	void eui_elem_ch_handler(std::uint32_t id, void * data, bool(*cb)(void *), void * newData, bool(*newCb)(void *));
	void eui_elem_del_handler(std::uint32_t id, void * data, bool(*cb)(void *));
	std::size_t eui_elem_property_len(std::uint32_t id, const char * prop, std::size_t len);
	std::size_t eui_elem_property_get(std::uint32_t id, char * buf, std::size_t maxlen, const char * prop, std::size_t len);
	bool eui_elem_property_get_bool(std::uint32_t id, const char * prop, std::size_t len);
	void eui_elem_property_set(std::uint32_t id, const char * prop, std::size_t proplen, const char * val, std::size_t vallen);
	void eui_elem_property_set_bool(std::uint32_t id, const char * prop, std::size_t proplen, bool val);
	std::size_t eui_elem_attr_len(std::uint32_t id, const char * prop, std::size_t len);
	std::size_t eui_elem_attr_get(std::uint32_t id, char * buf, std::size_t maxlen, const char * prop, std::size_t len);
	void eui_elem_attr_set(std::uint32_t id, const char * prop, std::size_t proplen, const char * val, std::size_t vallen);
	void eui_elem_attr_del(std::uint32_t id, const char * prop, std::size_t proplen);

	void eui_root_css_property_set(const char * prop, std::size_t proplen, const char * val, std::size_t vallen);
	void eui_root_attr_set(const char * prop, std::size_t proplen, const char * val, std::size_t vallen);

	void eui_elem_ss_del_rule(std::uint32_t id, std::size_t idx);
	void eui_elem_ss_del_rules(std::uint32_t id);
	void eui_elem_ss_ins_rule(std::uint32_t id, const char * rule, std::size_t rulelen, std::size_t idx);
	void eui_elem_ss_ins_rule_back(std::uint32_t id, const char * rule, std::size_t rulelen);

	void eui_get_vp_size(int * ww, int * wh);
	void eui_get_elem_size(std::uint32_t id, int * ew, int * eh);
	void eui_get_evt_pointer_coords(int * x, int * y, bool clamp2Win = false, int * ww = nullptr, int * wh = nullptr);

	void eui_wait_n_frames(int n, void * data, bool(*cb)(void *));

	std::size_t eui_blob_url_len(void);
	std::size_t eui_blob_url_from_buf(const std::uint8_t * buf, std::size_t len, const char * mime, std::size_t mimelen, char * urlbuf, std::size_t maxlen);
	void eui_blob_url_revoke(const char * url, std::size_t len);

	// JS -> C++
	bool eui_call_evt_handler(void * data, bool(*cb)(void *));
}

const char * eui_elem_selector(std::uint32_t id);
void eui_root_css_property_set(std::string_view prop, std::string_view val);
