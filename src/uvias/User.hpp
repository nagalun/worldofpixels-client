#pragma once

#include "util/explints.hpp"
#include "uvias/UviasRank.hpp"
#include <string>


class User {
public:
	using Id = u64;
	using Rep = i32;

private:
	const Id uid;
	std::string username;
	Rep totalRep;
	UviasRank rank;

public:
	User(Id);
	User(Id, Rep total, UviasRank, std::string);

	Id getId() const;
	Rep getTotalRep() const;
	const UviasRank& getUviasRank() const;
	const std::string& getUsername() const;

private:
	bool updateTotalRep(Rep);
	bool updateUser(std::string newName, UviasRank newRank);
	bool updateUser(UviasRank newRank);
};
