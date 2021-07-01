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
	u32 w;
	u32 h;
	u8 chans;

public:
	PngImage();
	PngImage(u8* filebuf, sz_t len);
	PngImage(u32 w, u32 h, RGB_u = {{255, 255, 255, 255}}, u8 chans = 4);

	u8 getChannels() const;
	u32 getWidth() const;
	u32 getHeight() const;
	const u8 * getData() const;
	u8 * getData();

	void applyTransform(std::function<RGB_u(u32 x, u32 y)>);
	RGB_u getPixel(u32 x, u32 y) const;
	void setPixel(u32 x, u32 y, RGB_u);
	void fill(RGB_u);

	void setChunkReader(const std::string&, std::function<bool(u8*, sz_t)>);
	void setChunkWriter(const std::string&, std::function<std::pair<std::unique_ptr<u8[]>, sz_t>()>);

	void allocate(u32 w, u32 h, RGB_u, u8 chans = 4);
	void readFileOnMem(const u8 * filebuf, sz_t len);
	void writeFileOnMem(std::vector<u8>& out);
	void nearestDownscale(u32 division);
	void freeMem();
};
