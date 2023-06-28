#pragma once

#include "color.hpp"
#include "explints.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <map>
#include <string>
#include <utility>

class PngImage {
	std::unique_ptr<u8[]> data;
	std::map<std::string, std::function<bool(u8*, sz_t)>> chunkReaders;
	std::map<std::string, std::function<std::pair<std::unique_ptr<u8[]>, sz_t>()>> chunkWriters;
	sz_t realBufSize;
	u32 w;
	u32 h;
	u8 chans;

public:
	PngImage();
	PngImage(u8* filebuf, sz_t len, bool stripAlpha = false, bool addAlpha = false);
	PngImage(u32 w, u32 h, RGB_u = {{255, 255, 255, 255}}, u8 chans = 4);

	u8 getChannels() const;
	u32 getWidth() const;
	u32 getHeight() const;
	const u8 * getData() const;
	u8 * getData();

	template<typename Fn> /* RGB_u(u32 x, u32 y) */
	void applyTransform(Fn func, bool blending = false) {
		for (u32 y = 0; y < h; y++) {
			for (u32 x = 0; x < w; x++) {
				setPixel(x, y, func(x, y), blending);
			}
		}
	}

	RGB_u getPixel(u32 x, u32 y) const;
	void setPixel(u32 x, u32 y, RGB_u, bool blending = false);
	void fill(RGB_u);
	void paste(u32 dstX, u32 dstY, const PngImage& src, bool blending = false, u32 srcX = 0, u32 srcY = 0);
	void paste(u32 dstX, u32 dstY, const PngImage& src, bool blending, u32 srcX, u32 srcY, u32 srcW, u32 srcH);
	void move(i32 offX, i32 offY);
	bool isFullyTransparent() const;

	std::pair<i32, i32> fitToContent();

	void setChunkReader(const std::string&, std::function<bool(u8*, sz_t)>);
	void setChunkWriter(const std::string&, std::function<std::pair<std::unique_ptr<u8[]>, sz_t>()>);

	PngImage clone() const;
	void resize(u32 nw, u32 nh);
	void allocate(u32 w, u32 h, RGB_u, u8 chans = 4, bool reuse = true, bool initialize = true);
	void readFileOnMem(const u8 * filebuf, sz_t len, bool stripAlpha = false, bool addAlpha = false);
	void writeFileOnMem(std::vector<u8>& out);
	void nearestDownscale(u32 division);
	void freeMem();
};
