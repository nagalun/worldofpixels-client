#pragma once

#include <set>

template<typename N>
class IdSys {
	N currentId;
	std::set<N> freeIds;

public:
	IdSys();

	N peekNextId() const;
	N getId();
	void freeId(N);

private:
	void shrink();
};
