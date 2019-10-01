#include "Bucket.hpp"

Bucket::Bucket(Bucket::Rate rate, Bucket::Per per)
: rate(rate),
  per(per),
  allowance(rate) { }

Bucket::Bucket(Bucket::Rate rate, Bucket::Per per, Bucket::Allowance allowance)
: rate(rate),
  per(per),
  allowance(allowance) { }

void Bucket::set(Bucket::Rate nrate, Bucket::Per nper) {
	rate = nrate;
	per = nper < 1 ? 1 : nper;
	if (allowance > nrate) {
		allowance = nrate;
	}
}

void Bucket::set(Bucket::Rate nrate, Bucket::Per nper, Bucket::Allowance nallowance) {
	rate = nrate;
	per = nper < 1 ? 1 : nper;
	allowance = nallowance;
}

bool Bucket::canSpend(Rate count) const {
	const auto now = std::chrono::steady_clock::now();
	std::chrono::duration<Allowance> passed = now - lastCheck;
	Allowance ace = allowance + passed.count() * (static_cast<Allowance>(rate) / per);

	if (ace > rate) {
		ace = rate;
	}

	return count <= ace;
}

bool Bucket::spend(Rate count) {
	const auto now = std::chrono::steady_clock::now();
	std::chrono::duration<Allowance> passed = now - lastCheck;
	lastCheck = now;
	allowance += passed.count() * (static_cast<Allowance>(rate) / per);

	if (allowance > rate) {
		allowance = rate;
	}

	if (allowance < count) {
		return false;
	}

	allowance -= count;
	return true;
}

Bucket::Rate Bucket::getRate() const {
	return rate;
}

Bucket::Per Bucket::getPer() const {
	return per;
}

Bucket::Allowance Bucket::getAllowance() const {
	return allowance;
}
