#pragma once

#ifdef DEBUG
#include <string>
#include <string_view>
#include <typeindex>

std::string_view demangle(std::type_index);
std::type_index strToType(std::string_view s);
#endif
