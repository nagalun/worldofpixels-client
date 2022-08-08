#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

extern "C" { // C++ -> JS
	std::uint32_t eui_create_elem(const char * tag, std::size_t len);
	void eui_elem_add_class(std::uint32_t id, const char * name, std::size_t len);
	void eui_elem_del_class(std::uint32_t id, const char * name, std::size_t len);
	void eui_append_elem(std::uint32_t parentid, std::uint32_t id);
	void eui_append_elem_selector(const char * parent, std::size_t len, std::uint32_t id);
	void eui_remove_elem(std::uint32_t id);
	void eui_destroy_elem(std::uint32_t id);
	void eui_elem_add_handler(std::uint32_t id, const char * evt, std::size_t len, void * data, bool(*cb)(void *));
	void eui_elem_ch_handler(std::uint32_t id, void * data, bool(*cb)(void *), void * newData, bool(*newCb)(void *));
	void eui_elem_del_handler(std::uint32_t id, void * data, bool(*cb)(void *));
	std::size_t eui_elem_property_len(std::uint32_t id, const char * prop, std::size_t len);
	std::size_t eui_elem_property_get(std::uint32_t id, const char * buf, std::size_t maxlen, const char * prop, std::size_t len);
	void eui_elem_property_set(std::uint32_t id, const char * prop, std::size_t proplen, const char * val, std::size_t vallen);
	void eui_elem_property_set_bool(std::uint32_t id, const char * prop, std::size_t proplen, bool val);
	std::size_t eui_elem_attr_len(std::uint32_t id, const char * prop, std::size_t len);
	std::size_t eui_elem_attr_get(std::uint32_t id, const char * buf, std::size_t maxlen, const char * prop, std::size_t len);
	void eui_elem_attr_set(std::uint32_t id, const char * prop, std::size_t proplen, const char * val, std::size_t vallen);
	void eui_root_css_property_set(const char * prop, std::size_t proplen, const char * val, std::size_t vallen);

	// JS -> C++
	bool eui_call_evt_handler(void * data, bool(*cb)(void *));
}

const char * eui_elem_selector(std::uint32_t id);
void eui_root_css_property_set(std::string_view prop, std::string_view val);
