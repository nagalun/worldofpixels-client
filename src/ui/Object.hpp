#pragma once

#include <string>
#include <string_view>
#include <cstdint>

#include "utils/NonCopyable.hpp"

namespace eui {

class Object : public NonCopyable {
	std::uint32_t id;

public:
	Object(); // creates a div by default
	Object(std::string_view tag);

	Object(Object&&);
	Object& operator=(Object&&);

	~Object();

	std::uint32_t getId() const;
	std::string_view getSelector() const;

	void addClass(std::string_view);
	void delClass(std::string_view);

	std::string getProperty(std::string_view name) const;
	void setProperty(std::string_view name, std::string_view value);

	void appendTo(std::string_view selector);
	void appendTo(std::uint32_t id);
	void appendTo(const Object&);

	void remove();

private:
	void destroy();
};

} // namespace eui
