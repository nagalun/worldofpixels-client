#include "util/emsc/dom.hpp"
#include "util/emsc/ui/Object.hpp"
#include <utility>

using namespace eui; // hm

Object::Object()
: Object("div") { }

Object::Object(std::string_view tag)
: id(eui_create_elem(tag.data(), tag.size())) { }

Object::Object(Object&& m) noexcept
: id(std::exchange(m.id, 0)) { }

Object& Object::operator =(Object&& m) noexcept {
	id = std::exchange(m.id, 0);
	return *this;
}

Object::~Object() {
	destroy();
}

std::uint32_t Object::getId() const {
	return id;
}

std::string_view Object::getSelector() const {
	return eui_elem_selector(getId());
}

void eui::Object::getOffsetSize(int *ow, int *oh) const {
	eui_get_elem_size(getId(), ow, oh);
}


void Object::addClass(std::string_view cl) {
	eui_elem_add_class(id, cl.data(), cl.size());
}

bool Object::tglClass(std::string_view cl) {
	return eui_elem_tgl_class(id, cl.data(), cl.size());
}

void Object::delClass(std::string_view cl) {
	eui_elem_del_class(id, cl.data(), cl.size());
}

std::string Object::getProperty(std::string_view name) const {
	std::size_t len = eui_elem_property_len(id, name.data(), name.size());
	std::string buf(len, '\0');
	eui_elem_property_get(id, buf.data(), buf.size() + 1, name.data(), name.size());

	return buf;
}

bool Object::getPropertyBool(std::string_view name) const {
	return eui_elem_property_get_bool(id, name.data(), name.size());
}

void Object::setProperty(std::string_view name) {
	setProperty(name, name);
}

void Object::setProperty(std::string_view name, std::string_view value) {
	eui_elem_property_set(id, name.data(), name.size(), value.data(), value.size());
}

void Object::setPropertyBool(std::string_view name, bool value) {
	eui_elem_property_set_bool(id, name.data(), name.size(), value);
}

std::string Object::getAttribute(std::string_view name) const {
	std::size_t len = eui_elem_attr_len(id, name.data(), name.size());
	std::string buf(len, '\0');
	eui_elem_attr_get(id, buf.data(), buf.size() + 1, name.data(), name.size());

	return buf;
}

void Object::delAttribute(std::string_view name) {
	eui_elem_attr_del(id, name.data(), name.size());
}

void Object::setAttribute(std::string_view name) {
	setAttribute(name, name);
}

void Object::setAttribute(std::string_view name, std::string_view value) {
	eui_elem_attr_set(id, name.data(), name.size(), value.data(), value.size());
}

EventHandle Object::createHandler(std::string_view name, std::function<bool(void)> cb, bool passive) {
	return EventHandle(getId(), name, std::move(cb), passive);
}

EventHandle Object::createWindowHandler(std::string_view name, std::function<bool(void)> cb, bool passive) {
	return EventHandle(0, name, std::move(cb), passive);
}

void Object::appendTo(std::string_view selector) {
	eui_append_elem_selector(selector.data(), selector.size(), id);
}

void Object::appendTo(std::uint32_t targetId) {
	eui_append_elem(targetId, id);
}

void Object::appendTo(const Object& o) {
	eui_append_elem(o.getId(), id);
}

void Object::appendToMainContainer() {
	appendTo("#eui-container");
}

void Object::appendToHead() {
	appendTo("head");
}

void Object::remove() {
	eui_remove_elem(id);
}

void Object::destroy() {
	if (id != 0) {
		eui_destroy_elem(id);
	}
}
