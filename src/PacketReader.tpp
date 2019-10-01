#include <tuple>

#include <cstdio>
#include <string>
#include "demangler.hpp"

template<typename Packet, typename Func>
void PacketReader::on(Func f) {
	std::fputs("Registered packet ", stdout);
	std::fputs(demangle(typeid(Packet)).c_str(), stdout);
	std::fputs(" (", stdout);
	std::fputs(std::to_string(u16(Packet::code)).c_str(), stdout);
	std::fputs(")\n", stdout);
	handlers.emplace(Packet::code, [f{std::move(f)}] (const u8 * data, sz_t size) {
		//std::fputs("Parsing opc ", stdout);
		//std::puts(std::to_string(u16(Packet::code)).c_str());

		std::apply(f, Packet::fromBuffer(data, size));
	});
}
