#include "InputManager.hpp"

#include <algorithm>
#include <memory>

Keybind::Keybind(std::array<std::vector<std::string>, 2> keys, EKeybindTriggers trg)
: keys(std::move(keys)),
  trg(trg) { }

Keybind::Keybind(EKeybindTriggers trg)
: trg(trg) { }

EKeybindTriggers Keybind::getTriggers() const {
	return trg;
}

bool Keybind::isMixed() const {
	return !keys[0].empty() && !keys[1].empty();
}

// loose or strict match depending on parameter's .isMixed()
// isPress = current event is a keypress?
// wasPress = last event was a keypress?
bool Keybind::canActivate(const Keybind& k, bool isPress, bool wasPress) {
	bool check = false;
	// if key released and we're listening for releases
	if (!isPress && (k.trg & K_ONRELEASE)) {
		// if the last ev was a release, fire only if we're listening for
		// presses too
		check = wasPress || (k.trg & K_ONPRESS);
	}

	if (!check) {
		return false;
	}

	return k.isMixed()
			? *this == k
			: (!k.keys[1].empty() && keys[1] == k.keys[1]) || keys[0] == k.keys[0];
}

// always strict checking
bool Keybind::operator ==(const Keybind& kb) const {
	return keys == kb.keys;
}

bool Keybind::operator  <(const Keybind& kb) const {
	return keys < kb.keys;
}



ImAction::ImAction(std::string ctx, std::string name, std::function<bool(const Keybind&, bool))> cb)
: context(std::move(ctx)),
  name(std::move(name)),
  cb(std::move(cb)),
  enabled(true),
  bindable(true) { }

const std::string& ImAction::getContext() const {
	return context;
}

const std::string& ImAction::getName() const {
	return name;
}

bool ImAction::isEnabled() const {
	return enabled;
}

void ImAction::setEnabled(bool s) {
	enabled = s;
}

void ImAction::setCb(std::function<bool(const Keybind&, bool)> cb) {
	this->cb = std::move(cb);
}

bool ImAction::operator()(const Keybind& k, bool isPressed) {
	return cb(k, isPressed);
}



ImActionProxy::ImActionProxy(InputManager& im)
: a(nullptr),
  im(im) { }

ImActionProxy::ImActionProxy(ImAction& a, InputManager& im)
: a(std::addressof(a)),
  im(im) { }

ImActionProxy::ImActionProxy(ImActionProxy&& iap)
: a(iap.a),
  im(iap.im) {
	iap.a = nullptr;
}

ImActionProxy::~ImActionProxy() {
	if (a) {
		im.del(*a);
	}
}

ImAction * ImActionProxy::getAction() const {
	return a;
}


InputManager::InputManager(std::string targetElem) {
	std::printf("[InputManager] Initialized. Target element: %s\n", targetElem.c_str());
}

ImActionProxy InputManager::add(std::string ctx, std::string name,
		Keybind defaultKb, std::function<bool(const Keybind&, bool))> cb) {
	auto customKeybindIt = pendingRestores.find(std::make_pair(ctx, name));

	auto ok = actions.emplace(std::move(ctx), std::move(name), std::move(cb));
	ImAction& ac = *ok.first;
	if (!ok.second) {
		std::printf("[InputManager] Duplicate Action?!: %s/%s\n",
				ac.getContext().c_str(), ac.getName().c_str());
		return *this;
	}

	if (customKeybindIt != pendingRestores.end()) {
		bindings.insert_or_assign(std::move(customKeybindIt->second), std::ref(ac));
		pendingRestores.erase(customKeybindIt);
	} else {
		bindings.insert_or_assign(std::move(defaultKb), std::ref(ac));
	}

	return {ac, *this};
}

void InputManager::del(ImAction& a) {
	auto it = std::find_if(bindings.begin(), bindings.end(), [&a] (const auto& e) {
		return a == e.second;
	});

	if (it != bindings.end()) {
		pendingRestores.insert_or_assign(std::make_pair(a.getContext(), a.getName()), it->first);
		bindings.erase(it);
	}

	actions.erase(a);
}

bool InputManager::setKeybinding(std::string ctx, std::string name, Keybind kb) {
	auto it = std::find_if(bindings.begin(), bindings.end(), [&ctx, &name] (const auto& e) {
		return ctx == e.second.getContext() && name == e.second.getName();
	});

	if (it == bindings.end()) {
		pendingRestores.insert_or_assign(std::make_pair(std::move(ctx), std::move(name)), std::move(kb));
	} else {
		auto binding = bindings.extract(it);
		binding.key() = std::move(kb);
		bindings.insert(std::move(binding));
	}

	return true;
}
