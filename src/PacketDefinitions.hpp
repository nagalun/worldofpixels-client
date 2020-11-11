#pragma once

#include <util/Bucket.hpp>
#include <util/net/Packet.hpp>
#include <uvias/User.hpp>
#include <uvias/UviasRank.hpp>
#include <world/Cursor.hpp>
#include <world/World.hpp>
#include <optional>


namespace net {
// to client
enum tc : u8 {
	AUTH_PROGRESS,
	AUTH_OK,
	AUTH_ERROR,
	PLAYER_DATA,
	USER_UPDATE,
	SHOW_PLAYERS,
	HIDE_PLAYERS,
	WORLD_DATA,
	WORLD_UPDATE,
	TOOL_STATE,
	CHAT_MESSAGE,
	PROTECTION_UPD,
	STATS

	/*TELEPORT, // use player data for this?
	PERMISSIONS,
	SET_PQUOTA*/
};

using Cursor = std::tuple<Cursor::Id, World::Pos, World::Pos, Cursor::Step, Cursor::Tid>;
using Pixel  = std::tuple<World::Pos, World::Pos, u8, u8, u8>;
using Bucket = std::tuple<Bucket::Rate, Bucket::Per, Bucket::Allowance>;

} // namespace net

// Packet definitions, clientbound
using AuthProgress = Packet<net::tc::AUTH_PROGRESS, std::string>;
// uid, username, total rep, rank id, rank name, super user, can self manage
using AuthOk       = Packet<net::tc::AUTH_OK,       User::Id, std::string, User::Rep, UviasRank::Id, std::string, bool, bool>;
using AuthError    = Packet<net::tc::AUTH_ERROR,    std::string>;

// self cursor data, paint bucket, chat bucket, can chat, can modify world
using CursorData  = Packet<net::tc::PLAYER_DATA,  net::Cursor, net::Bucket, net::Bucket, bool, bool>;
using UserUpdate  = Packet<net::tc::USER_UPDATE,  User::Id>;
using CursorsShow = Packet<net::tc::SHOW_PLAYERS, std::vector<std::tuple<User::Id, net::Cursor>>>;
using CursorsHide = Packet<net::tc::HIDE_PLAYERS, std::vector<Cursor::Id>>;

// world name, motd, bg color, drawing restricted, owner
using WorldData        = Packet<net::tc::WORLD_DATA,     std::string, std::string, u32, bool, std::optional<User::Id>>;
using WorldUpdate      = Packet<net::tc::WORLD_UPDATE,   std::vector<net::Cursor>, std::vector<net::Pixel>>;
//using ToolState        = Packet<net::tc::TOOL_STATE,     Cursor::Id, >
using ChatMessage      = Packet<net::tc::CHAT_MESSAGE,   User::Id, std::string>;
using ProtectionUpdate = Packet<net::tc::PROTECTION_UPD, Chunk::ProtPos, Chunk::ProtPos, u32>;
using Stats            = Packet<net::tc::STATS,          u32, u32>;

// Packet definitions, serverbound

