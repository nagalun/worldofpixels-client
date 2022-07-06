#include "demangler.hpp"
#ifdef DEBUG
#include <map>

//#ifdef __GNUG__
//	#define HAVE_CXA_DEMANGLE
//	#include <memory>
//	#include <cxxabi.h>
//#endif

std::map<std::type_index, std::string, std::less<>> typeCache;
std::map<std::string, std::type_index, std::less<>> typeMap;

std::string_view demangle(std::type_index type) { // XXX: also not thread safe
	auto search = typeCache.find(type);

	if (search == typeCache.end()) {
#ifdef HAVE_CXA_DEMANGLE
		int s = -1;
		std::unique_ptr<char, void(*)(void*)> result(
			abi::__cxa_demangle(type.name(), NULL, NULL, &s),
			std::free
		);

		std::string name(s == 0 ? result.get() : type.name());
#else
		std::string name(type.name());
#endif
		search = typeCache.emplace(type, name).first;
		typeMap.emplace(name, type);
	}

	return search->second;
}

std::type_index strToType(std::string_view s) {
	auto search = typeMap.find(s);
	if (search == typeMap.end()) {
		return typeid(void);
	}

	return search->second;
}
#endif
