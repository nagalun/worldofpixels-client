#pragma once

#include <map>
#include <functional>

#include "explints.hpp"

class PacketReader {
	using OpCode = u8;
	std::map<OpCode, std::function<void(const u8 *, sz_t)>> handlers;

public:
	PacketReader();

	bool read(const u8 *, sz_t);

	template<typename Packet, typename Func>
	void on(Func);
};

#include "PacketReader.tpp"
