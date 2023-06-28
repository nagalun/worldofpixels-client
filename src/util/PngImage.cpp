#include <algorithm>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <exception>
#include <tuple>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <png.h>
#include "PngImage.hpp"
#include "util/color.hpp"

// inspiration from: https://gist.github.com/DanielGibson/e0828acfc90f619198cb

struct img_t {
	std::unique_ptr<u8[]> data;
	u32 w;
	u32 h;
	u8 chans;
};

static void pngError(png_structp pngPtr, png_const_charp msg) {
	puts(msg);
	//std::abort();
}

static void pngWarning(png_structp pngPtr, png_const_charp msg) {
	puts(msg);
}

static void * pngMalloc(png_structp pngPtr, png_size_t length) {
	// new calls the oom handler, unlike malloc
	return ::operator new(length);
}

static void pngFree(png_structp pngPtr, void * ptr) {
	::operator delete(ptr);
}

static void pngReadData(png_structp pngPtr, png_bytep data, png_size_t length) {
	u8* b = (u8*)png_get_io_ptr(pngPtr);
	std::copy_n(b, length, data);

	png_set_read_fn(pngPtr, b + length, pngReadData); // XXX: hmmm.............
}

static int pngReadChunkCb(png_structp pngPtr, png_unknown_chunkp chunk) {
	auto* map(static_cast<std::map<std::string, std::function<bool(u8*, sz_t)>>*>(png_get_user_chunk_ptr(pngPtr)));
	std::string key(reinterpret_cast<char*>(chunk->name), 4);
	auto search = map->find(key);
	if (search != map->end()) {
		return search->second(chunk->data, chunk->size) ? 1 : -1;
	}
	return 0;
}

static struct img_t loadPng(const u8* fbuffer, int len,
		std::map<std::string, std::function<bool(u8*, sz_t)>>& chunkReaders,
		bool stripAlpha = false, bool addAlpha = false) {
	// create png_struct with the custom error handlers
	png_structp pngPtr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, nullptr, pngError, pngWarning,
			nullptr, pngMalloc, pngFree);
	if (!pngPtr) {
		puts("loadPng: png_create_read_struct failed");
		std::terminate();
	}

	// allocate the memory for image information
	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr) {
		png_destroy_write_struct(&pngPtr, nullptr);
		puts("loadPng: png_create_info_struct failed");
		std::terminate();
	}

	png_set_read_fn(pngPtr, const_cast<u8 *>(fbuffer), pngReadData);
	png_set_read_user_chunk_fn(pngPtr, &chunkReaders, pngReadChunkCb);
	png_set_sig_bytes(pngPtr, 0);
	png_read_info(pngPtr, infoPtr);

	png_uint_32 pngWidth, pngHeight;
	int bitDepth, colorType, interlaceType;
	png_get_IHDR(pngPtr, infoPtr, &pngWidth, &pngHeight, &bitDepth, &colorType, &interlaceType, nullptr, nullptr);

	// 16 bit -> 8 bit
	png_set_strip_16(pngPtr);

	// 1, 2, 4 bit -> 8 bit
	if (bitDepth < 8) {
		png_set_packing(pngPtr);
	}

	if (colorType & PNG_COLOR_MASK_PALETTE) {
		png_set_expand(pngPtr);
	}

	if (!(colorType & PNG_COLOR_MASK_COLOR)) {
		png_set_gray_to_rgb(pngPtr);
	}

	u8 chans = 3;
	if (colorType & PNG_COLOR_MASK_ALPHA) {
		chans++;
	}

	if (chans == 4 && stripAlpha) {
		png_set_strip_alpha(pngPtr); // remove alpha
		chans--;
	}

	if (chans == 3 && addAlpha) {
		png_set_add_alpha(pngPtr, 0xFF, PNG_FILLER_AFTER);
		chans++;
	}

	png_read_update_info(pngPtr, infoPtr);

	auto out(std::make_unique<u8[]>(pngWidth * pngHeight * chans));

	//png_uint_32 rowBytes = png_get_rowbytes(pngPtr, infoPtr);

	std::vector<png_bytep> rowPointers(pngHeight);
	for (png_uint_32 row = 0; row < pngHeight; row++) {
		rowPointers[row] = (png_bytep)(out.get() + row * pngWidth * chans);
	}

	png_read_image(pngPtr, rowPointers.data());

	png_read_end(pngPtr, infoPtr);
	png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);

	return {std::move(out), pngWidth, pngHeight, chans};
}

static void pngWriteDataToMem(png_structp png_ptr, png_bytep data, png_size_t length) {
	std::vector<u8>* p = (std::vector<u8>*)png_get_io_ptr(png_ptr);
	p->insert(p->end(), data, data + length);
}

static void encodePng(size_t pngWidth, size_t pngHeight, u8 chans, const u8* data,
		void(*iofun)(png_structp, png_bytep, png_size_t), void* iodata,
		std::map<std::string, std::function<std::pair<std::unique_ptr<u8[]>, sz_t>()>>& chunkWriters) {
	png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, pngError, pngWarning);
	if (!pngPtr) {
		puts("encodePng: png_create_read_struct failed");
		std::terminate();
	}

	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr) {
		png_destroy_write_struct(&pngPtr, nullptr);
		puts("encodePng: png_create_info_struct failed");
		std::terminate();
	}

	sz_t actualChunkCount = 0;
	std::vector<std::unique_ptr<u8[]>> toDelete(chunkWriters.size());
	std::vector<png_unknown_chunk_t> chunkArr(chunkWriters.size());

	for (auto& chunk : chunkWriters) {
		png_unknown_chunkp unk = &chunkArr[actualChunkCount];
		auto ret(chunk.second());
		if (!ret.first) {
			// skip, if the writer returned a null buffer
			continue;
		}

		std::copy_n(chunk.first.data(), 4, unk->name);
		unk->data = ret.first.get();
		unk->size = ret.second;
		unk->location = PNG_HAVE_PLTE;
		// make the buffer live till the end of this function
		toDelete[actualChunkCount] = std::move(ret.first);
		actualChunkCount++;
	}

	png_set_IHDR(pngPtr, infoPtr, pngWidth, pngHeight, 8,
			chans == 4 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);

	//png_set_compression_level(pngPtr, 4);

	std::vector<png_bytep> rowPointers(pngHeight);
	for (png_uint_32 row = 0; row < pngHeight; row++) {
		rowPointers[row] = (png_bytep)(data + (row * pngWidth * chans));
	}

	png_set_rows(pngPtr, infoPtr, rowPointers.data());
	png_set_write_fn(pngPtr, iodata, iofun, nullptr);

	png_write_info_before_PLTE(pngPtr, infoPtr);

	if (actualChunkCount > 0) {
		png_set_unknown_chunks(pngPtr, infoPtr, chunkArr.data(), actualChunkCount);
	}

	png_write_info(pngPtr, infoPtr);
	png_write_image(pngPtr, rowPointers.data());
	png_write_end(pngPtr, infoPtr);

	png_destroy_write_struct(&pngPtr, &infoPtr);
}

PngImage::PngImage()
: data(nullptr),
  realBufSize(0),
  w(0),
  h(0),
  chans(4) { }

PngImage::PngImage(u8* filebuf, sz_t len, bool stripAlpha, bool addAlpha) {
	readFileOnMem(filebuf, len, stripAlpha, addAlpha);
}

PngImage::PngImage(u32 w, u32 h, RGB_u bg, u8 chans) {
	allocate(w, h, bg, chans);
}

u8 PngImage::getChannels() const {
	return chans; // can only be 3 or 4 (RGB/RGBA)
}

u32 PngImage::getWidth() const {
	return w;
}

u32 PngImage::getHeight() const {
	return h;
}

const u8 * PngImage::getData() const {
	return data.get();
}

u8 * PngImage::getData() {
	return data.get();
}

RGB_u PngImage::getPixel(u32 x, u32 y) const {
	u8 * d = data.get();
	u8 c = getChannels();
	return {{
		d[(y * w + x) * c],
		d[(y * w + x) * c + 1],
		d[(y * w + x) * c + 2],
		c == 4 ? d[(y * w + x) * c + 3] : u8(255)
	}};
}

void PngImage::setPixel(u32 x, u32 y, RGB_u clr, bool blending) {
	u8 * d = data.get();
	u8 c = getChannels();
	u8 defaultAlpha = 255;
	u8 sR = clr.c.r;
	u8 sG = clr.c.g;
	u8 sB = clr.c.b;
	u8 sA = clr.c.a;
	u8 * dR = &d[(y * w + x) * c];
	u8 * dG = &d[(y * w + x) * c + 1];
	u8 * dB = &d[(y * w + x) * c + 2];
	u8 * dA = c == 4 ? &d[(y * w + x) * c + 3] : &defaultAlpha;
	if (!blending || sA == 255) {
		*dR = sR;
		*dG = sG;
		*dB = sB;
		*dA = sA;
		return;
	}

	if (sA == 0 && *dA == 0) {
		return;
	}

	float sRf = sR / 255.f;
	float sGf = sG / 255.f;
	float sBf = sB / 255.f;
	float sAf = sA / 255.f;
	float dRf = *dR / 255.f;
	float dGf = *dG / 255.f;
	float dBf = *dB / 255.f;
	float dAf = *dA / 255.f;

	float fAf = sAf + dAf * (1.f - sAf);
	float fRf = (sAf * sRf + dAf * dRf * (1.f - sAf)) / fAf;
	float fGf = (sAf * sGf + dAf * dGf * (1.f - sAf)) / fAf;
	float fBf = (sAf * sBf + dAf * dBf * (1.f - sAf)) / fAf;

	*dR = std::round(std::min(fRf, 1.f) * 255.f);
	*dG = std::round(std::min(fGf, 1.f) * 255.f);
	*dB = std::round(std::min(fBf, 1.f) * 255.f);
	*dA = std::round(std::min(fAf, 1.f) * 255.f);
}

void PngImage::fill(RGB_u clr) {
	u8 * d = data.get();
	u8 c = getChannels();
	for (sz_t i = 0; i < w * h * c; i++) {
		d[i] = (u8) (clr.rgb >> ((i % c) * 8));
	}
}

void PngImage::paste(u32 dstX, u32 dstY, const PngImage& src, bool blending, u32 srcX, u32 srcY) {
	paste(dstX, dstY, src, blending, srcX, srcY, src.getWidth(), src.getHeight());
}

void PngImage::paste(u32 dstX, u32 dstY, const PngImage& src, bool blending, u32 srcX, u32 srcY, u32 srcW, u32 srcH) {
	if (dstX >= w || dstY >= h || srcX >= src.getWidth() || srcY >= src.getHeight()) {
		return;
	}

	u32 endX = dstX + srcW;
	u32 endY = dstY + srcH;
	u32 endSX = srcX + srcW;
	u32 endSY = srcY + srcH;
	endX = endX > w ? w : endX;
	endY = endY > h ? h : endY;
	endSX = endSX > src.getWidth() ? src.getWidth() : endSX;
	endSY = endSY > src.getHeight() ? src.getHeight() : endSY;
	for (u32 y = dstY, sy = srcY; y < endY && sy < endSY; y++, sy++) {
		for (u32 x = dstX, sx = srcX; x < endX && sx < endSX; x++, sx++) {
			setPixel(x, y, src.getPixel(sx, sy), blending);
		}
	}
}

void PngImage::move(i32 offX, i32 offY) { // is it worth the complexity to avoid the alloc & clone?
	if (offX == 0 && offY == 0) {
		return;
	}

	PngImage tmp(clone());
	applyTransform([&, this](u32 x, u32 y) -> RGB_u {
		i32 newX = static_cast<i32>(x) - offX;
		i32 newY = static_cast<i32>(y) - offY;
		RGB_u clr =
			newX < 0 || u32(newX) >= w || newY < 0 || u32(newY) >= h ? RGB_u{.rgb = 0} : tmp.getPixel(newX, newY);
		return clr;
	});
}

bool PngImage::isFullyTransparent() const {
	if (getChannels() != 4) { return false; }

	u8 * d = data.get();
	for (std::size_t i = 0; i < w * h * 4; i += 4) {
		if (d[i + 3] != 0) {
			return false;
		}
	}

	return true;
}

std::pair<i32, i32> PngImage::fitToContent() {
	if (getChannels() != 4) { return {0, 0}; }

	u32 nw = w;
	u32 nh = h;
	i32 xoff = 0;
	i32 yoff = 0;

	// reduce top side of the image
	for (u32 y = 0, x, i = 0; y < h; y++) {
		for (x = 0; x < w && i == 0; i += getPixel(x++, y).c.a);
		if (i) { break; }
		yoff--;
		nh--;
	}

	// reduce left side of the image
	for (u32 x = 0, y, i = 0; x < w; x++) {
		for (y = -yoff; y < h && i == 0; i += getPixel(x, y++).c.a);
		if (i) { break; }
		xoff--;
		nw--;
	}

	// reduce bottom side of the image
	for (i32 y = h - 1, x, i = 0; y >= -yoff; y--) {
		for (x = w; x-- > -xoff && i == 0; i += getPixel(x, y).c.a);
		if (i) { break; }
		nh--;
	}

	// reduce right side of the image
	for (i32 x = w - 1, y, i = 0; x >= -xoff; x--) {
		for (y = nh + -yoff; y-- > -yoff && i == 0; i += getPixel(x, y).c.a);
		if (i) { break; }
		nw--;
	}

	move(xoff, yoff);
	resize(nw, nh);

	return {xoff, yoff};
}

void PngImage::setChunkReader(const std::string& s, std::function<bool(u8*, sz_t)> f) {
	chunkReaders[s] = std::move(f);
}

void PngImage::setChunkWriter(const std::string& s, std::function<std::pair<std::unique_ptr<u8[]>, sz_t>()> f) {
	chunkWriters[s] = std::move(f);
}

PngImage PngImage::clone() const {
	PngImage c;
	c.w = w;
	c.h = h;
	c.chans = chans;
	if (data) {
		c.data = std::make_unique<u8[]>(w * h * chans);
		c.realBufSize = w * h * chans;
		std::memcpy(c.data.get(), data.get(), w * h * chans);
	}

	return c;
}

void PngImage::resize(u32 newWidth, u32 newHeight) {
	if (w == newWidth && h == newHeight) { return; }
	if (newWidth == 0 || newHeight == 0) {
		data = nullptr;
		realBufSize = 0;
		w = 0;
		h = 0;
		return;
	}

	if (realBufSize < newWidth * newHeight * chans) {
		auto newData = std::make_unique<u8[]>(newWidth * newHeight * chans);
		std::memcpy(newData.get(), data.get(), w * h * chans);
		data = std::move(newData);
		realBufSize = newWidth * newHeight * chans;
	}

	u8 * d = data.get();
	if (newWidth < w) {
		// skips y = 0
		u8 * prevRowEnd = &d[newWidth * chans];
		for (u32 y = 1; y < h; y++) {
			u8 * currRowStart = &d[y * w * chans];
			std::memmove(prevRowEnd, currRowStart, newWidth * chans);
			prevRowEnd += newWidth * chans;
		}
	} else if (newWidth > w) {
		// same as above but in reverse order
		// skips y = 0
		u8 * destRowStart = &d[(newHeight - 1) * newWidth * chans];
		for (u32 y = h - 1; y > 0; y--) {
			u8 * currRowStart = &d[y * w * chans];
			std::memmove(destRowStart, currRowStart, w * chans);
			// fill the new pixels with zeroes
			std::memset(destRowStart + w * chans, 0, (newWidth - w) * chans);
			destRowStart -= newWidth * chans;
		}
		// fill new pixels on y = 0
		std::memset(&d[w * chans], 0, (newWidth - w) * chans);
	}

	if (newHeight > h) {
		// fill the new pixels with zeroes. more height is at the end so no skipping is necessary
		std::memset(&d[h * newWidth * chans], 0, (newHeight - h) * newWidth * chans);
	}

	w = newWidth;
	h = newHeight;
}

void PngImage::allocate(u32 newWidth, u32 newHeight, RGB_u bg, u8 newChans, bool reuse, bool initialize) {
	if (newWidth != w || newHeight != h || newChans != chans || !data.get()) {

		if (!reuse || !data.get() || realBufSize < newWidth * newHeight * newChans) {
			data = std::make_unique<u8[]>(newWidth * newHeight * newChans);
			realBufSize = newWidth * newHeight * newChans;
		}

		w = newWidth;
		h = newHeight;
		chans = newChans;
	}

	if (initialize) {
		fill(bg);
	}
}

void PngImage::readFileOnMem(const u8 * filebuf, sz_t len, bool stripAlpha, bool addAlpha) {
	auto img(loadPng(filebuf, len, chunkReaders, stripAlpha, addAlpha));
	data = std::move(img.data);
	w = img.w;
	h = img.h;
	chans = img.chans;
	realBufSize = w * h * chans;
}

void PngImage::writeFileOnMem(std::vector<u8>& out) {
	out.clear();
	encodePng(w, h, getChannels(), data.get(), pngWriteDataToMem, static_cast<void*>(&out), chunkWriters);
}

void PngImage::nearestDownscale(u32 division) {
	u32 newW = w / division;
	u32 newH = h / division;
	PngImage newData(newW, newH);
	for (u32 y = 0; y < newH; y++) {
		for (u32 x = 0; x < newW; x++) {
			newData.setPixel(x, y, getPixel(x / newW * w, y / newH * h));
		}
	}

	w = newW;
	h = newH;
	data = std::move(newData.data);
}

void PngImage::freeMem() {
	data = nullptr;
	w = 0;
	h = 0;
}
