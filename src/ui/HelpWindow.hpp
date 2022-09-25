#pragma once

#include <cstdint>

#include <util/emsc/ui/Window.hpp>
#include <util/emsc/ui/Object.hpp>

class HelpWindow : public eui::Window {
	eui::Object linkCont;
	eui::Object discordLnk;
	eui::Object fbookLnk;
	eui::Object redditLnk;
	eui::Object ppalLnk;
	eui::Object gpediaLnk;
	eui::Object helpCont;

public:
	HelpWindow();
};

