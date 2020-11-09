#pragma once

#include <cstdint>

extern "C" { // C++ -> JS
	std::uint32_t eui_create_elem(const char * tag, std::size_t len);
	void eui_elem_add_class(std::uint32_t id, const char * name, std::size_t len);
	void eui_elem_del_class(std::uint32_t id, const char * name, std::size_t len);
	void eui_append_elem(std::uint32_t parentid, std::uint32_t id);
	void eui_append_elem_selector(const char * parent, std::size_t len, std::uint32_t id);
	void eui_remove_elem(std::uint32_t id);
	void eui_destroy_elem(std::uint32_t id);
	std::size_t eui_elem_property_len(std::uint32_t id, const char * prop, std::size_t len);
	std::size_t eui_elem_property_get(std::uint32_t id, char * buf, std::size_t maxlen, const char * prop, std::size_t len);
	std::size_t eui_elem_property_set(std::uint32_t id, char * prop, std::size_t proplen, char * val, std::size_t vallen);
}

const char * eui_elem_selector(std::uint32_t id);
