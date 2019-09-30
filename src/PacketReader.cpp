#include "PacketReader.hpp"

PacketReader::PacketReader() { }

bool PacketReader::read(const u8 * buf, sz_t size) {
	auto search = handlers.find(buf[0]);
	
	if (search != handlers.end()) {
		search->second(buf + 1, size - 1);
		return true;
	}

	return false;
}
