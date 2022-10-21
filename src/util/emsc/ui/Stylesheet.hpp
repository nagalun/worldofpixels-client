#pragma once

#include <cstdint>
#include <string_view>

#include "util/emsc/ui/Object.hpp"

namespace eui {

class Stylesheet : public Object {
public:
	Stylesheet();

	std::size_t ruleCount() const;
	void deleteAllRules();

	void deleteRule(std::size_t i);
	void insertRule(std::string_view, std::size_t i = 0);
	void setDisabled(bool);
};

} /* namespace eui */
