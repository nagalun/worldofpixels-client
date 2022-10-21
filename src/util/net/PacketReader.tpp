#include <tuple>

template<typename Packet, typename Func>
void PacketReader::on(Func f) {
	handlers.emplace(Packet::code, [f{std::move(f)}] (const u8 * data, sz_t size) {
		//std::fputs("Parsing opc ", stdout);
		//std::puts(std::to_string(u16(Packet::code)).c_str());

		std::apply(f, Packet::fromBuffer(data, size));
	});
}
