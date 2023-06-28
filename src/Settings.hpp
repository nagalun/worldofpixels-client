#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>

#include "util/Signal.hpp"
#include "uvias/User.hpp"

struct Settings {
	template <typename T>
	class Param : Signal<void(T)> {
		T value;

	public:
		Param() { }

		Param(T&& value)
		: value(value) { }

		const T& get() const {
			return value;
		}

		operator const T&() const {
			return value;
		}

		template <typename AT>
		const T& operator=(AT&& newVal) {
			value = std::forward<AT>(newVal);
			Signal<void(T)>::fire(value);
			return value;
		}

		using Type = T;
		using Signal<void(T)>::connect;
		using Signal<void(T)>::setBlocked;
		using SlotKey = typename Signal<void(T)>::SlotKey;
	};

	// general
	Param<bool> showGrid{true};
	Param<bool> invertClrs{false};
	Param<bool> showProtectionZones{true};
	Param<bool> hideAllPlayers{false};
	Param<bool> nativeRes{true};

	// audio
	Param<bool> enableAudio{true};
	Param<bool> enableWorldAudio{true};
	Param<float> joinSfxVol{1.f};
	Param<float> buttonSfxVol{1.f};
	Param<float> paintSfxVol{1.f};

	// theming
	Param<bool> enableWorldThemes{true};
	Param<std::string> selectedTheme{"default"};

	// social
	enum class AlertBehavior { Never, OnlyMentions, UnfocusedChat, UnfocusedTab };

	Param<bool> hideChat{false};
	Param<AlertBehavior> chatAlerts{AlertBehavior::OnlyMentions};
	Param<bool> muteAllGuests{false};
	Param<std::map<User::Id, std::int64_t>> mutes;

	static Settings& get();
};
