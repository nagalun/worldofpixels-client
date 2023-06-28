#pragma once

#include "util/explints.hpp"
#include <tuple>
#include <memory>
#include <vector>


template<u8 opCode, typename... Args>
struct Packet {
	static constexpr u8 code = opCode;
	using value_type = std::tuple<Args...>;

	Packet() = delete;
	//Packet(Args... args);

	// maybe could be changed to std::optional to avoid exceptions
	// NOTE: doesn't read opcode!
	static std::tuple<Args...> fromBuffer(const u8 * buffer, sz_t size);

	static std::tuple<std::unique_ptr<u8[]>, sz_t> toBuffer(const Args&... args);
	static void toBuffer(std::vector<u8>& out, const Args&... args);
	static sz_t toBuffer(u8* out, sz_t maxSize, const Args&... args);
};

template<typename F>
struct fromBufFromLambdaArgs : public fromBufFromLambdaArgs<decltype(&F::operator())> {};

template<typename ClassType, typename ReturnType, typename... Args>
struct fromBufFromLambdaArgs<ReturnType(ClassType::*)(Args...) const> {
	using value_type = std::tuple<Args...>;
	static std::tuple<Args...> call(const u8 * d, sz_t s) {
		return Packet<0, Args...>::fromBuffer(d, s);
	}
};

#include "Packet.tpp" // IWYU pragma: keep
