#include "misc.hpp"
#include <memory>

std::pair<char*, std::size_t> get_char_buf(std::size_t min_size) {
	static std::unique_ptr<char[]> buf = nullptr;
	static std::size_t size = 0;

	if (size < min_size) {
		buf = std::make_unique<char[]>(min_size);
		size = min_size;
	}

	return {buf.get(), size};
}
