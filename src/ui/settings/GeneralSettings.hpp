#pragma once

#include <cstdint>
#include <string_view>

#include <util/emsc/ui/Object.hpp>
#include <util/emsc/ui/EventHandle.hpp>
#include <ui/misc/Option.hpp>
#include <Settings.hpp>

class GeneralSettings : public eui::Object {
	using S = Settings;
	eui::Object hdrAppearance;
	LabelledOption<decltype(S::showGrid)> showGrid;
	LabelledOption<decltype(S::nativeRes)> nativeRes;
	LabelledOption<decltype(S::showProtectionZones)> showProtectionZones;
	LabelledOption<decltype(S::hideAllPlayers)> hideAllPlayers;
	eui::Object hdrAudio;
	LabelledOption<decltype(S::enableAudio)> enableAudio;
	LabelledOption<decltype(S::enableWorldAudio)> enableWorldAudio;
	LabelledOption<decltype(S::joinSfxVol)> joinSfxVol;
	LabelledOption<decltype(S::buttonSfxVol)> buttonSfxVol;
	LabelledOption<decltype(S::paintSfxVol)> paintSfxVol;

	eui::EventHandle onClickJoinVol;
	eui::EventHandle onClickButtonVol;
	eui::EventHandle onClickPaintVol;

public:
	GeneralSettings();

	std::string_view getTabName() const;
};

