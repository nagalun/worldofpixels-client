#include "BufferHelper.hpp"
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <cstdint>

namespace rle {

template<typename T>
std::pair<std::unique_ptr<u8[]>, sz_t> compress(T* arr, u16 numItems) {
	struct compressedPoint { u16 pos; u16 length; };
	// vector of points where data repeats
	std::vector<compressedPoint> compressedPos;

	sz_t compBytes = sizeof(T) * numItems;
	for (u16 i = 1, t = 0; i <= numItems; i++) {
		// true if we're at the end of the array
		if (i == numItems || arr[i] != arr[i - 1]) {
			sz_t saved = (sz_t(i - t) - 1) * sizeof(T);
			// if the bytes saved by shrinking the repeated values are more
			// than the size of the definition of the point itself then reduce.
			if (saved > sizeof(compressedPoint)) {
				// reduce the estimate of the compressed data and insert the
				// location of the reduction to the vector
				compBytes -= saved;
				compressedPos.push_back({t, u16(i - t)});
			}
			// set the last unique value's poisiton to the current index
			t = i;
		}
	}

	const sz_t points(compressedPos.size() * sizeof(compressedPoint));

	// 3 u16: original length, elem size, num repeats
	// + the size of the vector of points in bytes
	const sz_t header(sizeof(u16) * 3 + points);

	// allocate the buffer that the compressed data will reside in as a unique_ptr
	auto out(std::make_unique<u8[]>(header + compBytes));

	u8* curr = out.get();
	curr += buf::writeLE(curr, numItems);
	curr += buf::writeLE(curr, u16(sizeof(T)));
	curr += buf::writeLE(curr, u16(compressedPos.size()));

	curr = std::copy_n(
		reinterpret_cast<u8*>(compressedPos.data()),
		points,
		curr
	);

	T* currT = reinterpret_cast<T*>(curr);
	sz_t arrIdx = 0;
	for (auto point : compressedPos) {
		currT = std::copy(&arr[arrIdx], &arr[point.pos + 1], currT);
		arrIdx = point.pos + point.length;
	}
	currT = std::copy_n(&arr[arrIdx], numItems - arrIdx, currT);
	sz_t size = reinterpret_cast<u8*>(currT) - out.get();
	return {std::move(out), size};
}

template<typename T>
sz_t getItems(u8 * in, sz_t size) {
	if (size < sizeof(u16) * 3) {
		return 0;
	}

	u8* curr = in;
	u16 numItems = buf::readLE<u16>(curr); curr += sizeof(u16);
	u16 itemSize = buf::readLE<u16>(curr); curr += sizeof(u16);
	if (itemSize != sizeof(T)) {
		return 0;
	}

	return numItems;
}

template<typename T>
bool decompress(u8* in, sz_t inSize, T* output, sz_t outMaxItems) {
	struct cPoint { u16 pos; u16 length; };
	u8* curr = in;
	if (inSize < sizeof(u16) * 3) {
		return false;
	}

	u16 numItems = buf::readLE<u16>(curr); curr += sizeof(u16);
	u16 itemSize = buf::readLE<u16>(curr); curr += sizeof(u16);
	u16 numRptPoints = buf::readLE<u16>(curr); curr += sizeof(u16);
	if (itemSize != sizeof(T)) {
		return false;
	}

	if (numItems > outMaxItems) {
		return false;
	}

	if (numRptPoints * sizeof(cPoint) >= inSize - (curr - in)) {
		return false;
	}

	// WASM doesn't like unaligned accesses/stores, so let's align the data
	constexpr std::size_t alignment = std::max({alignof(u16), alignof(T)});
	static_assert(alignment <= sizeof(u16) * 3, "Alignment requirements too strict to decompress");
	if (std::size_t offset = reinterpret_cast<std::uintptr_t>(curr) % alignment) {
		std::move(curr, curr + (inSize - (curr - in)), curr - offset);
		curr -= offset;
	}

	cPoint* pt = reinterpret_cast<cPoint*>(curr);
	curr += numRptPoints * sizeof(cPoint);
	T* data = reinterpret_cast<T*>(curr);
	sz_t j = 0;
	sz_t k = 0;
	// this assumes that the compressed points in the array are ordered
	for (u16 i = 0; i < numRptPoints; i++, pt++) { // XXX: no bound checking!
		while (j < pt->pos) {
			output[j++] = data[k++];
		}
		std::fill_n(&output[j], pt->length, data[k++]);
		j += pt->length;
	}
	while (j < numItems) {
		output[j++] = data[k++];
	}

	return true;
}

}
