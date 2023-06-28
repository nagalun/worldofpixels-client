#pragma once
#include "PacketReader.hpp"
#include <tuple>
#include <type_traits>

template<typename Packet, typename Func>
requires std::is_same_v<typename Packet::value_type, typename fromBufFromLambdaArgs<Func>::value_type>
void PacketReader::on(Func f) {
	handlers.emplace(Packet::code, [f{std::move(f)}] (const u8 * data, sz_t size) {
		std::apply(f, Packet::fromBuffer(data, size));
	});
}
