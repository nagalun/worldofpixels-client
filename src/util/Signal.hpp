#pragma once

#include <vector>
#include <utility>

#include "SmallLambda.hpp"

template <typename T> class Signal;
template <typename... Args>
class Signal<void(Args...)> {
	using FtorClass = SmallLambda<void(Args...)>;
	template<typename... T>
	using Container = std::vector<T...>;

public:
	class SlotKey;

private:
	struct Connection {
		SlotKey * key;
		FtorClass cb;

		template<typename T>
		Connection(SlotKey * key, T&& callable)
		: key(key),
		  cb(std::forward<T>(callable)) { }

		Connection(const Connection&) = delete;
		const Connection& operator=(const Connection&) = delete;

		Connection(Connection&& o) noexcept
		: key(std::exchange(o.key, nullptr)),
		  cb(std::exchange(o.cb, nullptr)) { }

		const Connection& operator=(Connection&& o) noexcept {
			key = std::exchange(o.key, nullptr);
			cb = std::exchange(o.cb, nullptr);
			return *this;
		}

		~Connection() {
			del();
		}

		void operator=(std::nullptr_t) {
			del();
		}

		operator bool() const {
			return key;
		}

		void del() {
			if (key) {
				key->ref = nullptr;
				key = nullptr;
				cb = nullptr;
			}
		}
	};

	using Iter = typename Container<Connection>::iterator;

public:
	class SlotKey {
		Signal<void(Args...)> * ref;
		Iter it;

		SlotKey(Signal<void(Args...)> * ref, Iter it)
		: ref(ref),
		  it(it) {
			it->key = this;
		}

	public:
		SlotKey()
		: ref(nullptr) { }

		SlotKey(const SlotKey&) = delete;
		const SlotKey& operator=(const SlotKey&) = delete;

		SlotKey(SlotKey&& o) noexcept
		: ref(std::exchange(o.ref, nullptr)),
		  it(o.it) {
			it->key = this;
		}

		const SlotKey& operator=(SlotKey&& o) noexcept {
			ref = std::exchange(o.ref, nullptr);
			it = o.it;
			it->key = this;
			return *this;
		}

		~SlotKey() {
			disconnect();
		}

		operator bool() const {
			return ref;
		}

		template<typename T>
		void setCb(T&& callable) {
			if (ref) {
				it->cb = std::forward<T>(callable);
			}
		}

		void disconnect() {
			if (ref) {
				ref->disconnect(it);
				ref = nullptr;
			}
		}

		friend Connection;
		friend Signal<void(Args...)>;
	};

private:
	Container<Connection> slots;
	bool blocked;

public:
	Signal()
	: blocked(false) { }

	Signal(const Signal&) = delete;
	const Signal& operator=(const Signal&) = delete;

	Signal(Signal&& o)
	: blocked(o.blocked) {
		slots.swap(o.slots);
		updateKeyRefs(slots.begin());
	}

	const Signal& operator=(Signal&& o) {
		slots.clear();
		slots.swap(o.slots);
		blocked = o.blocked;
		updateKeyRefs(slots.begin());
	}

	template<typename T>
	SlotKey connect(T&& callable) {
		bool moved = slots.size() == slots.capacity();
		slots.emplace_back(nullptr, std::forward<T>(callable));
		if (moved) {
			updateKeyRefs(slots.begin());
		}

		return SlotKey{this, slots.end() - 1};
	}

	void operator()(Args... args) {
		if (isBlocked() || slots.empty()) {
			return;
		}

		auto it = slots.begin();
		for (; it != slots.end() - 1; ++it) {
			it->cb(args...);
		}

		it->cb(std::forward<Args>(args)...);
	}

	void fire(Args... args) {
		(*this)(std::forward<Args>(args)...);
	}

	void setBlocked(bool st) {
		blocked = st;
	}

	bool isBlocked() const {
		return blocked;
	}

private:
	void disconnect(Iter it) {
		updateKeyRefs(slots.erase(it));
	}

	void updateKeyRefs(Iter from) {
		for (auto it = from; it != slots.end(); ++it) {
			if (*it) {
				it->key->ref = this;
				it->key->it = it;
			}
		}
	}

	friend SlotKey;
};
