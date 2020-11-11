#pragma once

#include <string>
#include <typeindex>

const std::string& demangle(std::type_index);
std::type_index strToType(const std::string& s);
