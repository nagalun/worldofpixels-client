#include "util/emsc/ui/Stylesheet.hpp"
#include "util/emsc/dom.hpp"
#include <string_view>

namespace eui {

Stylesheet::Stylesheet()
: Object("style") { }

Stylesheet::Stylesheet(std::string_view url)
: Object("link") {
	setAttribute("rel", "stylesheet");
	setAttribute("href", url);
}

std::size_t Stylesheet::ruleCount() const {
	return getPropertyAs<std::uint32_t>("sheet.cssRules.length").value_or(0);
}

void Stylesheet::deleteAllRules() {
	eui_elem_ss_del_rules(getId());
}

void Stylesheet::deleteRule(std::size_t i) {
	eui_elem_ss_del_rule(getId(), i);
}

void Stylesheet::insertRule(std::string_view css, std::size_t i) {
	eui_elem_ss_ins_rule(getId(), css.data(), css.size(), i);
}

void Stylesheet::insertRuleBack(std::string_view css) {
	eui_elem_ss_ins_rule_back(getId(), css.data(), css.size());
}

void Stylesheet::setDisabled(bool s) {
	setPropertyBool("disabled", s);
}
} /* namespace eui */
