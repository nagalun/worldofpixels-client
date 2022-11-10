#include "BlobUrl.hpp"
#include "util/emsc/dom.hpp"

#include <memory>
#include <utility>

using namespace eui;

BlobUrl::BlobUrl(std::string url)
: url(std::move(url)) { }

BlobUrl::BlobUrl() { }

BlobUrl::BlobUrl(BlobUrl&& o) noexcept
: url(std::exchange(o.url, std::string{})) { }

const BlobUrl& BlobUrl::operator=(BlobUrl&& o) noexcept {
	url = std::exchange(o.url, std::string{});
	return *this;
}

BlobUrl::~BlobUrl() {
	if (!url.empty()) {
		eui_blob_url_revoke(url.c_str(), url.size());
	}
}

std::string_view eui::BlobUrl::get() const {
	return url;
}

BlobUrl BlobUrl::fromBuf(const std::uint8_t* buf, std::size_t len, std::string_view mimeType) {
	static std::size_t bufLen = eui_blob_url_len(); // it won't change, right?

	auto outBuf = std::make_unique<char[]>(bufLen);
	std::size_t outLen = eui_blob_url_from_buf(buf, len, mimeType.data(), mimeType.size(), outBuf.get(), bufLen);

	return BlobUrl(std::string(outBuf.get(), outLen));
}
