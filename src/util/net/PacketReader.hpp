#pragma once

#include "util/explints.hpp"
#include <unordered_map>
#include <functional>


class PacketReader {
	using OpCode = u8;
	std::unordered_map<OpCode, std::function<void(const u8 *, sz_t)>> handlers;

public:
	PacketReader();

	bool read(const u8 *, sz_t);

	template<typename Packet, typename Func>
	void on(Func);
};

#include "util/net/PacketReader.tpp" // IWYU pragma: keep
