#include "util/emsc/ui/Stylesheet.hpp"
#include "util/emsc/dom.hpp"

namespace eui {

Stylesheet::Stylesheet() { }

std::size_t Stylesheet::ruleCount() const {
	return getPropertyAs<std::uint32_t>("sheet.cssRules.length").value_or(0);
}

void Stylesheet::deleteRule(std::size_t i) {
	eui_elem_ss_del_rule(getId(), i);
}

void Stylesheet::insertRule(std::string_view css, std::size_t i) { }

void Stylesheet::setDisabled(bool s) {
	setPropertyBool("disabled", s);
}

void Stylesheet::deleteAllRules() { }
} /* namespace eui */
