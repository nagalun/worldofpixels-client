#include "IdSys.hpp"
#include <cstdint>

template<typename N>
IdSys<N>::IdSys()
: currentId(0) { }

template<typename N>
N IdSys<N>::peekNextId() const {
	return freeIds.empty() ? currentId + 1 : *freeIds.begin();
}

template<typename N>
N IdSys<N>::getId() {
	N id;
	if (!freeIds.empty()) {
		auto it = freeIds.begin();
		id = *it;
		freeIds.erase(it);
	} else {
		id = ++currentId;
	}

	return id;
}

template<typename N>
void IdSys<N>::freeId(N id) {
	if (id == currentId) {
		--currentId;
	} else {
		freeIds.emplace(id);
	}

	shrink();
}

template<typename N>
void IdSys<N>::shrink() {
	if (!freeIds.empty()) {
		auto it = freeIds.end();
		while (--it != freeIds.begin() && *it == currentId) {
			it = freeIds.erase(it);
			--currentId;
		}
	}
}

template class IdSys<std::uint32_t>;
