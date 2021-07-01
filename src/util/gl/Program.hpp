#pragma once

#include <string_view>
#include <initializer_list>
#include <cstdint>

namespace gl {

class Program {
	std::uint32_t id;

public:
	Program(std::string_view vs, std::string_view fs, std::initializer_list<const char *> attribOrder);
	~Program();

	Program(Program&&);
	Program& operator=(Program&&);
	Program(const Program &other) = delete;
	Program& operator=(const Program &other) = delete;

	void use() const;
	std::uint32_t get() const;
	std::int32_t findUniform(const char *) const;

	// is program valid
	operator bool() const;

	static std::uint32_t build(std::string_view vs, std::string_view fs, std::initializer_list<const char *> attribOrder);

private:
	void del();
};

} /* namespace gl */
