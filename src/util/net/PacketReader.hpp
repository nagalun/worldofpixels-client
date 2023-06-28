#pragma once

#include "util/explints.hpp"
#include <unordered_map>
#include <functional>
#include "Packet.hpp"

class PacketReader {
	using OpCode = u8;
	std::unordered_map<OpCode, std::function<void(const u8 *, sz_t)>> handlers;

public:
	PacketReader();

	bool read(const u8 *, sz_t);

	template<typename Packet, typename Func>
	requires std::is_same_v<typename Packet::value_type, typename fromBufFromLambdaArgs<Func>::value_type>
	void on(Func);
};

#include "util/net/PacketReader.tpp" // IWYU pragma: keep
