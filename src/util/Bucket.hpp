#pragma once

#include "util/explints.hpp"
#include <chrono>


class Bucket {
public:
	using Rate = u16;
	using Per = u16;
	using Allowance = float;

private:
	u16 rate;
	u16 per;
	Allowance allowance;
	std::chrono::steady_clock::time_point lastCheck;

public:
	Bucket(Rate, Per);
	Bucket(Rate, Per, Allowance);

	void set(Rate, Per);
	void set(Rate, Per, Allowance);

	bool canSpend(Rate = 1) const;
	bool spend(Rate = 1);

	Rate getRate() const;
	Per getPer() const;
	Allowance getAllowance() const;
};
