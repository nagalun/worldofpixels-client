#pragma once

#include "util/NonCopyable.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace eui {

class BlobUrl : NonCopyable {
	std::string url;

	BlobUrl(std::string);

public:
	BlobUrl();
	BlobUrl(BlobUrl&& o) noexcept;
	const BlobUrl& operator=(BlobUrl&& o) noexcept;
	~BlobUrl();

	std::string_view get() const;

	static BlobUrl fromBuf(const std::uint8_t* buf, std::size_t len, std::string_view mimeType);
};

}
