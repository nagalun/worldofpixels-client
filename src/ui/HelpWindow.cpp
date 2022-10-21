#include <ui/HelpWindow.hpp>
#include <string_view>

using namespace std::string_view_literals;

HelpWindow::HelpWindow()
: discordLnk("a"),
  fbookLnk("a"),
  redditLnk("a"),
  ppalLnk("a"),
  gpediaLnk("a") {
	setTitle("Help");

	using ilsv = std::initializer_list<std::string_view>;
	ilsv icons{"dcord"sv, "fbook"sv, "rddit"sv, "pypal"sv, "gpedia"sv};
	ilsv titles{"Discord Server"sv, "Facebook"sv, "Subreddit"sv, "Donate through PayPal"sv, "Gamepedia"sv};
	ilsv links{
		"https://owop.me/discord"sv,
		"https://www.facebook.com/OurWorldOfPixels/"sv,
		"https://reddit.com/r/OurWorldOfPixels"sv,
		"https://www.paypal.me/InfraRaven"sv,
		"https://ourworldofpixels.gamepedia.com/"sv
	};

	int i = 0;
	for (auto* lnk : {&discordLnk, &fbookLnk, &redditLnk, &ppalLnk, &gpediaLnk}) {
		lnk->addClass("owop-ui");
		lnk->setAttribute("data-i", *(icons.begin() + i));
		lnk->setAttribute("title", *(titles.begin() + i));
		lnk->setAttribute("href", *(links.begin() + i));
		lnk->setAttribute("target", "_blank");
		lnk->appendTo(linkCont);
		i++;
	}

	linkCont.addClass("help-links");
	helpCont.addClass("help-cont");
	linkCont.appendTo(getContent());
	helpCont.appendTo(getContent());

	addClass("owop-win-help");
}

