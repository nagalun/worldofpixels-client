#include "util/net/PacketReader.hpp"

PacketReader::PacketReader() { }

bool PacketReader::read(const u8 * buf, sz_t size) {
	OpCode opc(buf[0]);
	auto search = handlers.find(opc);
	
	if (search != handlers.end()) {
		search->second(buf + 1, size - 1);
		return true;
	}

	return false;
}
