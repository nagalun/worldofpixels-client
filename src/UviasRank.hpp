#pragma once

#include <string>
#include <string_view>

#include "explints.hpp"

class UviasRank {
public:
	using Id = i32;

private:
	std::string name;
	Id id;
	bool superUser;
	bool selfManage;

public:
	UviasRank(Id id, std::string name, bool superUser, bool selfManage);

	Id getId() const;
	std::string_view getName() const;
	bool isSuperUser() const;
	bool canSelfManage() const;

	bool deepEqual(const UviasRank&) const;

	bool operator ==(const UviasRank&) const;
};
