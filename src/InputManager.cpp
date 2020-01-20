#include "InputManager.hpp"

#include <algorithm>
#include <memory>
#include <cctype>

#include <emscripten/html5.h>

// TOTHINK: input "adapters" instead of contexts on actions?
// [Base, ActiveTool, UiCurrentWindow] (maybe sort by priority)
// adapters can cancel events, and link to other adapters

// maybe tool handlers should get selfcursor's mouse coords instead of using inputmanager's

Keybind::Keybind(std::vector<std::string> kbKeys, EPointerEvents mButtons)
: kbKeys(std::move(kbKeys)),
  mButtons(mButtons) { }

const std::vector<std::string>& Keybind::getKeyboardKeys() const {
	return kbKeys;
}

EPointerEvents Keybind::getPointerEvents() const {
	return mButtons;
}

bool Keybind::hasKbKeys() const {
	return !kbKeys.empty();
}

bool Keybind::isMixed() const {
	return mButtons && !kbKeys.empty();
}

// loose or strict match depending on parameter's .isMixed()
bool Keybind::canActivate(const Keybind& k) const {
	EPointerEvents m = k.getPointerEvents();
	if (!k.isMixed() && m) {
		return (getPointerEvents() & m) == m;
	}

	const std::vector<std::string>& binding = k.getKeyboardKeys();

	if (binding.size() > kbKeys.size() || kbKeys.empty()) {
		return false;
	}

	auto bit = binding.begin();
	auto kbit = std::find(kbKeys.begin(), kbKeys.end(), *bit);
	for (; kbit != kbKeys.end() && bit != binding.end(); ++kbit, ++bit) {
		if (*kbit != *bit) {
			return false;
		}
	}

	return k.isMixed()
		? kbit == kbKeys.end() && bit == binding.end() // keys are at the end of array
			&& (getPointerEvents() & m) == m
		: bit == binding.end();
}

bool Keybind::contains(const char * key) const {
	return std::find_if(kbKeys.begin(), kbKeys.end(), [key] (const auto& str) {
		return str == key;
	}) != kbKeys.end();
}

// always strict checking
bool Keybind::operator ==(const Keybind& kb) const {
	return mButtons == kb.mButtons && kbKeys == kb.kbKeys;
}

bool Keybind::operator  <(const Keybind& kb) const {
	if (kbKeys.size() < kb.kbKeys.size()) {
		return true;
	}

	if (kbKeys.size() > kb.kbKeys.size()) {
		return false;
	}

	if (kbKeys < kb.kbKeys) {
		return true;
	}

	if (kbKeys > kb.kbKeys) {
		return false;
	}

	return mButtons < kb.mButtons;
}



ImAction::ImAction(std::string name, u8 trg, InputAdapter& adapter,
		std::function<void(const Keybind&, EActionTriggers, const InputInfo&)> cb)
: name(std::move(name)),
  cb(std::move(cb)),
  adapter(adapter),
  enabled(true),
  bindable(true),
  trg((EActionTriggers)trg) { }

ImAction::~ImAction() {
	// The shared_ptr is considered expired when we get here, so, the weak_ptr
	// will not be able to .lock() this object. That's why we guess with .expired()
	adapter.del(name);
	std::printf("[~ImAction]\n");
}

const std::string& ImAction::getName() const {
	return name;
}

EActionTriggers ImAction::getTriggers() const {
	return trg;
}

bool ImAction::isEnabled() const {
	return enabled;
}

void ImAction::setEnabled(bool s) {
	enabled = s;
}

void ImAction::setCb(std::function<void(const Keybind&, EActionTriggers, const InputInfo&)> cb) {
	this->cb = std::move(cb);
}

bool ImAction::operator()(const Keybind& k, EActionTriggers activationType, const InputInfo& ii) {
	if (getTriggers() & activationType) {
		cb(k, activationType, ii);
		return true;
	}

	return false;
}

bool ImAction::operator ==(const ImAction& rhs) const {
	return name == rhs.name;
}

InputInfo::InputInfo()
: wheelDx(0.0),
  wheelDy(0.0),
  lastMouseX(0),
  lastMouseY(0),
  mouseX(0),
  mouseY(0) { }

double InputInfo::getWheelDx() const {
	return wheelDx;
}

double InputInfo::getWheelDy() const {
	return wheelDy;
}

int InputInfo::getLastMouseX() const {
	return lastMouseX;
}

int InputInfo::getLastMouseY() const {
	return lastMouseY;
}

int InputInfo::getMouseX() const {
	return mouseX;
}

int InputInfo::getMouseY() const {
	return mouseY;
}

void InputInfo::setWheel(double dx, double dy) {
	wheelDx = dx;
	wheelDy = dy;
}

void InputInfo::setMouse(int x, int y) {
	mouseX = x;
	mouseY = y;
}

void InputInfo::updateLastMouse() {
	lastMouseX = mouseX;
	lastMouseY = mouseY;
}


InputStorage::InputStorage() {
	std::printf("[InputStorage] TODO loading\n");
}

InputStorage::~InputStorage() {
	std::printf("[~InputStorage] TODO saving\n");
}

std::optional<Keybind> InputStorage::popStoredKeybind(const std::string& name) {
	auto it = savedBindings.find(name);
	if (it != savedBindings.end()) {
		Keybind kb = std::move(it->second);
		savedBindings.erase(it);
		return kb;
	}

	return std::nullopt;
}

void InputStorage::storeKeybind(std::string name, Keybind kb) {
	savedBindings.emplace(std::move(name), std::move(kb));
}


InputAdapter::InputAdapter(InputAdapter * parent, InputStorage& is, std::string context, int priority)
: parentAdapter(parent),
  storage(is),
  context(std::move(context)),
  priority(priority),
  enabled(true) {
	std::printf("[InputAdapter] Registered %s, prio %d\n", getFullContext().c_str(), priority);
}

InputAdapter::~InputAdapter() {
	std::printf("[~InputAdapter] Del %s\n", context.c_str());
	if (bindings.size() > 0) {
		std::printf("[~InputAdapter] %lu bindings still registered on adapter ", bindings.size());
		std::printf("%s!!\n", getFullContext().c_str()); // could segfault if any parent is deleted before this
	}
}

std::string InputAdapter::getFullContext() const {
	return parentAdapter ? parentAdapter->getFullContext() + '/' + context : context;
}

InputAdapter& InputAdapter::mkAdapter(std::string context, int priority) {
	auto ok = linkedAdapters.emplace(this, storage, std::move(context), priority);
	// No, the object's order won't change, c++.
	return const_cast<InputAdapter&>(*ok.first);
}

void InputAdapter::tick(const InputInfo& ii) const {
	for (const auto& adapter : linkedAdapters) {
		adapter.tick(ii);
	}

	for (const auto& binding : activeBindings) {
		// SLOW
		if (auto act = binding->second.lock()) {
			(*act)(binding->first, T_ONHOLD, ii);
		}
	}
}

bool InputAdapter::matchDown(const Keybind& currentKeys, const char * pressedKey,
		EActionTriggers lastTrigger, const InputInfo& ii) {
	if (!enabled) {
		return false;
	}

	for (auto it = linkedAdapters.rbegin(); it != linkedAdapters.rend(); ++it) {
		// TODO: change the set for a linked list or vector to get rid of cast
		InputAdapter& adapter = const_cast<InputAdapter&>(*it);
		if (adapter.matchDown(currentKeys, pressedKey, lastTrigger, ii)) {
			return true;
		}
	}

	// from more complex (long) bindings to less, prioritize
	for (auto it = bindings.rbegin(); it != bindings.rend(); it++) {
		EPointerEvents bindMbtns = it->first.getPointerEvents();

		if ((pressedKey
					? it->first.contains(pressedKey)
					: (bindMbtns & currentKeys.getPointerEvents()) == bindMbtns
						&& (!it->first.hasKbKeys() || it->first.isMixed()))
				&& std::find(activeBindings.begin(), activeBindings.end(), it) == activeBindings.end()
				&& currentKeys.canActivate(it->first)) {
			auto act = it->second.lock();
			if (!act || !act->isEnabled()) {
				continue;
			}

			if (act->getTriggers() & T_ONPRESS) {
				(*act)(it->first, T_ONPRESS, ii);
			}

			if (act->getTriggers() & (T_ONHOLD | T_ONRELEASE)) {
				activeBindings.emplace_back(it);
			}

			return true;
		}
	}

	return false;
}

bool InputAdapter::matchUp(const Keybind& currentKeys, const char * releasedKey,
		EActionTriggers lastTrigger, const InputInfo& ii) {
	bool consumed = false;

	for (auto it = linkedAdapters.rbegin(); it != linkedAdapters.rend(); ++it) {
		// TODO: change the set for a linked list or vector to get rid of cast
		InputAdapter& adapter = const_cast<InputAdapter&>(*it);
		consumed |= adapter.matchUp(currentKeys, releasedKey, lastTrigger, ii);
	}

	for (auto it = activeBindings.begin(); it != activeBindings.end();) {
		// double iterator!
		EPointerEvents bindMbtns = (*it)->first.getPointerEvents();

		if ((currentKeys.getPointerEvents() & bindMbtns) != bindMbtns
				|| (releasedKey && (*it)->first.contains(releasedKey))) {
			if (auto act = (*it)->second.lock()) {
				if (lastTrigger != T_ONRELEASE || (act->getTriggers() & T_ONPRESS)) {
					(*act)((*it)->first, T_ONRELEASE, ii);
				}
			}

			it = activeBindings.erase(it);
			consumed = true;
		} else {
			++it;
		}
	}

	return consumed;
}


std::shared_ptr<ImAction> InputAdapter::add(std::string name, Keybind defaultKb, u8 trg,
		std::function<void(const Keybind&, EActionTriggers, const InputInfo&)> cb) {
	std::string fullName(getFullContext() + '/' + name);
	std::optional<Keybind> customKeybind(storage.popStoredKeybind(fullName));

	auto act = std::make_shared<ImAction>(std::move(name), trg, *this, std::move(cb));
	auto ok = bindings.emplace(customKeybind.value_or(defaultKb), act);

	if (!ok.second) {
		auto act2 = ok.first->second.lock();
		std::printf("[InputAdapter] Duplicate keybind!!!: '%s' with '%s', (",
			fullName.c_str(), act2 ? act2->getName().c_str() : "[deleted]");
	} else {
		std::printf("[InputAdapter] Registered binding: %s (", fullName.c_str());
	}

	for (const std::string& key : ok.first->first.getKeyboardKeys()) {
		std::printf("%s + ", key.c_str());
	}

	std::printf("MB[%d])\n", ok.first->first.getPointerEvents());

	return act;
}

void InputAdapter::del(const std::string& name) {
	// to work correctly this function assumes that the actions are deleted one at a time
	std::string fullName(getFullContext() + '/' + name);
	for (auto it = bindings.begin(); it != bindings.end();) {
		if (it->second.expired()) {
			// TODO: ignore if default keybind
			storage.storeKeybind(fullName, it->first);
			it = bindings.erase(it);
		} else {
			++it;
		}
	}
}

bool InputAdapter::setKeybinding(const std::shared_ptr<ImAction>& act, Keybind kb) {
	auto it = std::find_if(bindings.begin(), bindings.end(), [&act] (const auto& e) {
		return !act.owner_before(e.second) && !e.second.owner_before(act);
	});

	if (it == bindings.end()) {
		bindings.emplace(std::move(kb), act);
	} else {
		auto binding = bindings.extract(it);
		binding.key() = std::move(kb);
		bindings.insert(std::move(binding));
	}

	return true;
}

bool InputAdapter::operator <(const InputAdapter& rhs) const {
	if (priority < rhs.priority) {
		return true;
	}

	if (priority > rhs.priority) {
		return false;
	}

	return context < rhs.context;
}




InputManager::InputManager(const char * targetElement)
: InputAdapter(nullptr, *this, "Base"),
  targetElement(targetElement),
  lastTrigger(T_ONPRESS) {
	emscripten_set_keydown_callback(targetElement, this, true, InputManager::handleKeyEvent);
	emscripten_set_keyup_callback(targetElement, this, true, InputManager::handleKeyEvent);
	emscripten_set_blur_callback(targetElement, this, true, InputManager::handleFocusEvent);

	emscripten_set_mousemove_callback(targetElement, this, true, InputManager::handleMouseEvent);
	emscripten_set_mousedown_callback(targetElement, this, true, InputManager::handleMouseEvent);
	emscripten_set_mouseup_callback(targetElement, this, true, InputManager::handleMouseEvent);

	emscripten_set_wheel_callback(targetElement, this, true, InputManager::handleWheelEvent);

	std::printf("[InputManager] Initialized. Target element: %s\n",
			targetElement == EMSCRIPTEN_EVENT_TARGET_WINDOW ? "window" : targetElement);
}

InputManager::~InputManager() {
	emscripten_clear_interval(tickIntervalId);

	emscripten_set_keydown_callback(targetElement, nullptr, true, nullptr);
	emscripten_set_keyup_callback(targetElement, nullptr, true, nullptr);
	emscripten_set_blur_callback(targetElement, nullptr, true, nullptr);

	emscripten_set_mousemove_callback(targetElement, nullptr, true, nullptr);
	emscripten_set_mousedown_callback(targetElement, nullptr, true, nullptr);
	emscripten_set_mouseup_callback(targetElement, nullptr, true, nullptr);

	emscripten_set_wheel_callback(targetElement, nullptr, true, nullptr);
	std::printf("[~InputManager]\n");
}

void InputManager::printHeldKeys() const {
	std::printf("[");
	for (std::size_t i = 0; i < kbKeys.size(); i++) {
		if (i != 0) {
			std::printf(", ");
		}

		std::printf("%s", kbKeys[i].c_str());
	}

	std::printf("]\n");
}

void InputManager::tick() {
	InputAdapter::tick(*this);
	InputInfo::updateLastMouse();
}

bool InputManager::keyDown(const char * key) {
	std::printf("[InputManager] KEYDOWN: key=%s kbKeys=", key);

	bool handled = false;
	auto it = std::find(kbKeys.begin(), kbKeys.end(), key);
	if (it == kbKeys.end()) {
		kbKeys.emplace_back(key);
		printHeldKeys();
		handled = matchDown(*this, key, lastTrigger, *this);
	} else {
		printHeldKeys();
	}

	lastTrigger = T_ONPRESS;

	return handled;
}

bool InputManager::keyUp(const char * key) {
	std::printf("[InputManager] KEYUP: key=%s kbKeys=", key);

	bool handled = false;
	auto it = std::find(kbKeys.begin(), kbKeys.end(), key);
	if (it != kbKeys.end()) {
		kbKeys.erase(it);
		printHeldKeys();
		handled = matchUp(*this, key, lastTrigger, *this);
	} else {
		printHeldKeys();
	}

	lastTrigger = T_ONRELEASE;

	return handled;
}

bool InputManager::mouseDown(int changed, int buttons) {
	std::printf("[InputManager] MDOWN: changes=%d buttons=%d\n", changed, buttons);

	mButtons = static_cast<EPointerEvents>(buttons);

	matchDown(*this, nullptr, lastTrigger, *this);

	return false;
}

bool InputManager::mouseUp(int changed, int buttons) {
	std::printf("[InputManager] MUP: changes=%d buttons=%d\n", changed, buttons);

	mButtons = static_cast<EPointerEvents>(buttons);

	matchUp(*this, nullptr, lastTrigger, *this);

	return false;
}

bool InputManager::mouseMove(int x, int y) {
	//std::printf("[InputManager] MMOVE: x=%d y=%d\n", x, y);
	InputInfo::setMouse(x, y);
	return false;
}

bool InputManager::wheel(double dx, double dy, [[maybe_unused]] int type) {
	//std::printf("[InputManager] WHEEL: dx=%f dy=%f dz=%f type=%d\n", dx, dy, dz, type);
	InputInfo::setWheel(dx, dy);
	mButtons = (EPointerEvents)(mButtons | P_MWHEEL);

	matchDown(*this, nullptr, lastTrigger, *this);

	mButtons = (EPointerEvents)(mButtons & ~P_MWHEEL);

	matchUp(*this, nullptr, lastTrigger, *this);

	return false;
}

void InputManager::lostFocus() {
	std::printf("[InputManager] BLUR\n");
	// release all keys in order to cancel release triggers
	while (kbKeys.size()) {
		keyUp(kbKeys[0].c_str());
	}
}

int InputManager::handleKeyEvent(int type, const EmscriptenKeyboardEvent * ev, void * data) {
	InputManager * im = static_cast<InputManager *>(data);

	if (ev->repeat) {
		return true;
	}

	// Uppercase the key, since a and A are considered different keys...
	// TODO: fix stuck key on case: hold 2, hold shift, release 2, release shift (no KEYUP is fired for 2)
	char * key = const_cast<char *>(ev->key); // XXX: risky
	for (int i = 0; i < 32 && key[i]; i++) {
		key[i] = std::toupper((unsigned char)key[i]);
	}

	switch (type) {
		case EMSCRIPTEN_EVENT_KEYDOWN:
			return im->keyDown(ev->key);

		case EMSCRIPTEN_EVENT_KEYUP:
			return im->keyUp(ev->key);
	}

	return false;
}

int InputManager::handleMouseEvent(int type, const EmscriptenMouseEvent * ev, void * data) {
	InputManager * im = static_cast<InputManager *>(data);

	switch (type) {
		case EMSCRIPTEN_EVENT_MOUSEMOVE:
			return im->mouseMove(ev->clientX, ev->clientY);

		case EMSCRIPTEN_EVENT_MOUSEDOWN:
			return im->mouseDown(ev->button, ev->buttons);

		case EMSCRIPTEN_EVENT_MOUSEUP:
			return im->mouseUp(ev->button, ev->buttons);
	}

	return false;
}

int InputManager::handleFocusEvent(int type, const EmscriptenFocusEvent *, void * data) {
	InputManager * im = static_cast<InputManager *>(data);

	switch (type) {
		case EMSCRIPTEN_EVENT_BLUR:
			im->lostFocus();
			return true;
	}

	return false;
}

int InputManager::handleWheelEvent(int type, const EmscriptenWheelEvent * ev, void * data) {
	InputManager * im = static_cast<InputManager *>(data);

	switch (type) {
		case EMSCRIPTEN_EVENT_WHEEL:
			if (ev->mouse.shiftKey) {
				// Invert X and Y if shift is held, nice feature... I think?
				return im->wheel(ev->deltaY, ev->deltaX, ev->deltaMode);
			} else {
				return im->wheel(ev->deltaX, ev->deltaY, ev->deltaMode);
			}
	}

	return false;
}
