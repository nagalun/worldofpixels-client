#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cctype>
#include <map>
#include <string>

#include "Client.hpp"

void e(std::map<u8, std::function<void(std::string)>>& m) {
	m.emplace(u8(0), [] (std::string ee) {
		std::puts(ee.c_str());
	});
}

void test() {
	std::map<u8, std::function<void(std::string)>> tmap;
	e(tmap);
	std::string ewr("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	std::string er("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	std::string wr("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	std::string ew("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	std::string rwr("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	u8 trash[60] = {0};
	tmap.find(0)->second(ewr + "hello");
}

int main() {
	Client c;
	//test();

	c.open("main");
	//test();
}
