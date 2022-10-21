#pragma once

#include <string_view>

bool playAudioId(std::string_view);
bool setVolumeAudioId(std::string_view, float);
void setAudioEnabled(bool);
bool getAudioEnabled();
