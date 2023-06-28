#include "uvias/User.hpp"

#include "util/misc.hpp"

User::User(User::Id uid)
: uid(uid),
  username(n2hexstr(uid)),
  totalRep(0) { }

User::User(User::Id uid, User::Rep totalRep, UviasRank rank, std::string u)
: uid(uid),
  username(std::move(u)),
  totalRep(totalRep),
  rank(rank) { }

User::Id User::getId() const { return uid; }
User::Rep User::getTotalRep() const { return totalRep; }
const UviasRank& User::getUviasRank() const { return rank; }
const std::string& User::getUsername() const { return username; }

bool User::updateUser(std::string newName, UviasRank newRank) {
	bool changed = false;
	if (username != newName) {
		changed = true;
		username = std::move(newName);
	}

	if (updateUser(std::move(newRank))) {
		// this function already notified the change for us
		return true;
	}

/*	if (changed) {
		for (auto session : linkedSessions) {
			session.get().userWasUpdated();
		}
	}*/

	return changed;
}

bool User::updateUser(UviasRank newRank) {
	bool changed = false;

	if (!rank.deepEqual(newRank)) {
		changed = true;
		rank = std::move(newRank);
	}

/*	if (changed) {
		for (auto session : linkedSessions) {
			session.get().userWasUpdated();
		}
	}*/

	return changed;
}
