#pragma once

#include "util/Bucket.hpp"
#include "util/net/Packet.hpp"
#include "uvias/User.hpp"
#include "uvias/UviasRank.hpp"
#include <optional>

namespace net {
// to client
enum C : u8 {
	C_AUTH_OK,
	C_SELF_PLAYER,
	C_USER_UPDATED,
	C_UPDT_PLAYERS,
	C_TOOL_ACTIONS,
	C_USER_INFO,
	C_WORLD_DATA,
	C_WORLD_UPDATE,
	C_SYSTEM_MESSAGE,
	C_CHAT_MESSAGE,
	C_STATS,
	C_SUBSCRIBED_AREAS

	/*TELEPORT, // use player data for this?
	PERMISSIONS,
	SET_PQUOTA*/
};

// to server
enum S : u8 {
	S_PLAYER_UPDATE,
	S_DO_TOOL_ACTION,
	S_GET_USER_BY_UID,
	S_SUBSCRIBE_AREA
};

// Network tool IDs
enum ToolId : u8 {
	TID_PENCIL,
	TID_MOVE,
	TID_ZOOM,
	TID_PIPETTE,
	TID_FILL,
	TID_EXPORT,
	TID_RULER,
	TID_UNKNOWN = 255
};

using DPlayerId = uvar; // Player::Id
using DAbsWPos = ivar; // World::Pos
using DRelWPos = ivar; // World::Pos
using DAbsUpdAreaPos = ivar;
using DToolState = uvar;
using DStateSyncSeq = u8;
using DActionSyncSeq = u8;
using DAreaSyncSeq = u8;
using DPlayerStep = u8;
using DPlayerTid = u8;

// uid, username, total rep, rank id, rank name, super user, can self manage
using UviasUser  = std::tuple<User::Id, std::string, User::Rep, UviasRank::Id, std::string, bool, bool>;
// pid, abs/rel x, abs/rel y, step, tool id
template<typename WPos>
using PlayerUpd  = std::tuple<DPlayerId, WPos, WPos, DPlayerStep, DPlayerTid, DToolState>;
template<typename WPos>
using ToolAction = std::tuple<DPlayerId, WPos, WPos, DPlayerTid, DToolState>;
using Bucket     = std::tuple<Bucket::Rate, Bucket::Per, Bucket::Allowance>;

using VPlayersShow = std::vector<std::tuple<User::Id, net::PlayerUpd<net::DAbsWPos>>>;
using VPlayersHide = std::vector<net::DPlayerId>;
using VPlayersUpdate = std::vector<net::PlayerUpd<net::DRelWPos>>;

} // namespace net

// Packet definitions, clientbound
using CAuthOk      = Packet<net::C_AUTH_OK,     net::UviasUser>;

// self player data, paint bucket, chat bucket, can chat, can modify world, state seq, action seq
using CPlayerData  = Packet<net::C_SELF_PLAYER,  net::PlayerUpd<net::DAbsWPos>, net::Bucket, net::Bucket, bool, bool, net::DStateSyncSeq, net::DActionSyncSeq>;
using CUserUpdate  = Packet<net::C_USER_UPDATED, User::Id>;
using CPlayersUpdt = Packet<net::C_UPDT_PLAYERS, net::DAbsUpdAreaPos, net::DAbsUpdAreaPos, net::VPlayersHide, net::VPlayersShow, net::VPlayersUpdate>;
// absolute tool actions vector will contain items when the receiver may not know who the player id is, useful when the action spans multiple update regions
using CToolActions = Packet<net::C_TOOL_ACTIONS, std::vector<net::ToolAction<net::DAbsWPos>>, std::vector<net::ToolAction<net::DRelWPos>>>;
using CUserInfo    = Packet<net::C_USER_INFO,    net::UviasUser>;

// world name, motd, bg color, read only, owner
using CWorldData        = Packet<net::C_WORLD_DATA,       std::string, std::string, u32, bool, std::optional<User::Id>>;
// category, is private, message
using CSystemMessage    = Packet<net::C_SYSTEM_MESSAGE,   std::string, bool, std::string>;
// (uid, pid)?, channel, is private, message
using CChatMessage      = Packet<net::C_CHAT_MESSAGE,     std::optional<std::tuple<User::Id, net::DPlayerId>>, std::string, bool, std::string>;
// pcount global, pcount world
using CStats            = Packet<net::C_STATS,            uvar, uvar>;
// area seq, areas
using CSubscribedAreas  = Packet<net::C_SUBSCRIBED_AREAS, net::DAreaSyncSeq, std::vector<std::tuple<net::DAbsUpdAreaPos, net::DAbsUpdAreaPos>>>;

// Packet definitions, serverbound
using SPlayerUpdate  = Packet<net::S_PLAYER_UPDATE,   net::PlayerUpd<net::DAbsWPos>, net::DStateSyncSeq>;
using SDoToolAction  = Packet<net::S_DO_TOOL_ACTION,  net::ToolAction<net::DAbsWPos>, net::DStateSyncSeq, net::DActionSyncSeq>;
using SGetUserByUid  = Packet<net::S_GET_USER_BY_UID, User::Id>;
// x, y, sub(1)/unsub(0)
using SSubscribeArea = Packet<net::S_SUBSCRIBE_AREA,  net::DAbsUpdAreaPos, net::DAbsUpdAreaPos, bool>;
