#pragma once

#include "Packet.hpp"
//#include "User.hpp"
//#include "UviasRank.hpp"
//#include "Cursor.hpp"
//#include "World.hpp"

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
	WORLD_UPDATE,
	TOOL_STATE,
	CHAT_MESSAGE,
	PROTECTION_UPD,
	STATS

	/*TELEPORT, // use player data for this?
	PERMISSIONS,
	SET_PQUOTA*/
};

//using Cursor = std::tuple<Cursor::Id, World::Pos, World::Pos, Cursor::Step, Cursor::Tid>;
//using Pixel  = std::tuple<World::Pos, World::Pos, u8, u8, u8>;

} // namespace net

// Packet definitions, clientbound
using AuthProgress = Packet<net::tc::AUTH_PROGRESS, std::string>;
// world name, uid, username, total rep, rank id, rank name, super user, can self manage
/*using AuthOk       = Packet<net::tc::AUTH_OK,       std::string, User::Id, std::string, User::Rep, UviasRank::Id, std::string, bool, bool>;
using AuthError    = Packet<net::tc::AUTH_ERROR,    std::string>;

using PlayerData  = Packet<net::tc::PLAYER_DATA,  net::Cursor>; // self player data
using UserUpdate  = Packet<net::tc::USER_UPDATE,  User::Id>;
using PlayersShow = Packet<net::tc::SHOW_PLAYERS, std::vector<std::tuple<User::Id, net::Cursor>>>;
using PlayersHide = Packet<net::tc::HIDE_PLAYERS, std::vector<Cursor::Id>>;

using WorldUpdate      = Packet<net::tc::WORLD_UPDATE,   std::vector<net::Cursor>, std::vector<net::Pixel>>;
//using ToolState        = Packet<net::tc::TOOL_STATE,     Cursor::Id, >
using ChatMessage      = Packet<net::tc::CHAT_MESSAGE,   User::Id, std::string>;
using ProtectionUpdate = Packet<net::tc::PROTECTION_UPD, Chunk::ProtPos, Chunk::ProtPos, u32>;
using Stats            = Packet<net::tc::STATS,          u32, u32>;
*/
// Packet definitions, serverbound

