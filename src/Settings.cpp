#include "Settings.hpp"

Settings& Settings::get() {
	static Settings s;
	return s;
}
