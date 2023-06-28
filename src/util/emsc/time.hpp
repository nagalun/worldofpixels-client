#pragma once

#include <chrono>

// returns as seconds
double getTime(bool update = false);
std::chrono::steady_clock::time_point getStClock(bool update = false);
