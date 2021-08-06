#include <algorithm>
#include <stdexcept>
#include <exception>
#include <tuple>
#include <cstdio>
#include <cstdlib>

#include <png.h>
#include "PngImage.hpp"

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
		std::map<std::string, std::function<bool(u8*, sz_t)>>& chunkReaders) {
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

	/*if (chans == 4) {
		png_set_strip_alpha(pngPtr);// remove alpha
		chans--;
	}*/

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
			PNG_COLOR_TYPE_RGB,
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
  w(0),
  h(0),
  chans(4) { }

PngImage::PngImage(u8* filebuf, sz_t len) {
	readFileOnMem(filebuf, len);
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

void PngImage::applyTransform(std::function<RGB_u(u32 x, u32 y)> func) {
	for (u32 y = 0; y < h; y++) {
		for (u32 x = 0; x < w; x++) {
			setPixel(x, y, func(x, y));
		}
	}
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

void PngImage::setPixel(u32 x, u32 y, RGB_u clr) {
	u8 * d = data.get();
	u8 c = getChannels();
	d[(y * w + x) * c] = clr.c.r;
	d[(y * w + x) * c + 1] = clr.c.g;
	d[(y * w + x) * c + 2] = clr.c.b;
	if (c == 4) {
		d[(y * w + x) * c + 3] = clr.c.a;
	}

}

void PngImage::fill(RGB_u clr) {
	u8 * d = data.get();
	u8 c = getChannels();
	for (sz_t i = 0; i < w * h * c; i++) {
		d[i] = (u8) (clr.rgb >> ((i % c) * 8));
	}
}

void PngImage::setChunkReader(const std::string& s, std::function<bool(u8*, sz_t)> f) {
	chunkReaders[s] = std::move(f);
}

void PngImage::setChunkWriter(const std::string& s, std::function<std::pair<std::unique_ptr<u8[]>, sz_t>()> f) {
	chunkWriters[s] = std::move(f);
}

void PngImage::allocate(u32 newWidth, u32 newHeight, RGB_u bg, u8 newChans) {
	data = std::make_unique<u8[]>(newWidth * newHeight * newChans);
	w = newWidth;
	h = newHeight;
	chans = newChans;
	fill(bg);
}

void PngImage::readFileOnMem(const u8 * filebuf, sz_t len) {
	auto img(loadPng(filebuf, len, chunkReaders));
	data = std::move(img.data);
	w = img.w;
	h = img.h;
	chans = img.chans;
}

void PngImage::writeFileOnMem(std::vector<u8>& out) {
	out.clear();
	encodePng(w, h, getChannels(), data.get(), pngWriteDataToMem, static_cast<void*>(&out), chunkWriters);
}

void PngImage::nearestDownscale(u32 division) {
	float newW = w / division;
	float newH = h / division;
	PngImage newData(newW, newH);
	for (float y = 0; y < newH; y++) {
		for (float x = 0; x < newW; x++) {
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

