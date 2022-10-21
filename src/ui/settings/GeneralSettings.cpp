#include "ui/settings/GeneralSettings.hpp"

#include "util/emsc/audio.hpp"

GeneralSettings::GeneralSettings()
: hdrAppearance("h1"),
  showGrid(S::get().showGrid, "Show grid"),
  nativeRes(S::get().nativeRes, "Native resolution"),
  showProtectionZones(S::get().showProtectionZones, "Show protection zones"),
  hideAllPlayers(S::get().hideAllPlayers, "Hide all players"),
  hdrAudio("h1"),
  enableAudio(S::get().enableAudio, "Enable audio"),
  enableWorldAudio(S::get().enableWorldAudio, "Enable custom world audio"),
  joinSfxVol(S::get().joinSfxVol, "Join SFX Volume"),
  buttonSfxVol(S::get().buttonSfxVol, "Button SFX Volume"),
  paintSfxVol(S::get().paintSfxVol, "Paint SFX Volume") {

	hdrAppearance.setProperty("textContent", "Appearance");
	hdrAppearance.appendTo(*this);
	showGrid.appendTo(*this);
	nativeRes.appendTo(*this);
	showProtectionZones.appendTo(*this);
	hideAllPlayers.appendTo(*this);

	hdrAudio.setProperty("textContent", "Audio");
	hdrAudio.appendTo(*this);
	enableAudio.appendTo(*this);
	enableWorldAudio.appendTo(*this);
	joinSfxVol.appendTo(*this);
	buttonSfxVol.appendTo(*this);
	paintSfxVol.appendTo(*this);

	joinSfxVol.getOpt().asRange(0.f, 1.f, 0.01f);
	buttonSfxVol.getOpt().asRange(0.f, 1.f, 0.01f);
	paintSfxVol.getOpt().asRange(0.f, 1.f, 0.01f);

	onClickJoinVol = joinSfxVol.createHandler("click", [] {
		playAudioId("a-join");
		return false;
	});

	onClickButtonVol = buttonSfxVol.createHandler("click", [] {
		playAudioId("a-btn");
		return false;
	});

	onClickPaintVol = paintSfxVol.createHandler("click", [] {
		playAudioId("a-pixel");
		return false;
	});;
}

std::string_view GeneralSettings::getTabName() const {
	return "General";
}
