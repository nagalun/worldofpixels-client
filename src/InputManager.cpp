#include "InputManager.hpp"

#include <algorithm>
#include <memory>
#include <cctype>
#include <cstdio>

#include <emscripten/html5.h>

// maybe tool handlers should get selfcursor's mouse coords instead of using inputmanager's

Keybind::Keybind(EKeyModifiers mods, std::string button)
: button(std::move(button)),
  mods(mods) { }

Keybind::Keybind(EKeyModifiers mods, const char * button)
: Keybind(mods, std::string(button)) { }

Keybind::Keybind(EKeyModifiers mods, EPointerButtons button)
: mods(mods) {
	switch (button) {
		case P_MPRIMARY:
			this->button = "LCLICK";
			break;

		case P_MSECONDARY:
			this->button = "RCLICK";
			break;

		case P_MMIDDLE:
			this->button = "MCLICK";
			break;

		case P_MFOURTH: // not the best names
			this->button = "4CLICK";
			break;

		case P_MFIFTH:
			this->button = "5CLICK";
			break;

		default:
			// throw not possible
			std::fprintf(stderr, "[Keybind] Trying to make a keybind with multiple/no mouse buttons! (%d)\n", button);
			break;
	}
}

Keybind::Keybind(std::string button)
: Keybind(M_NONE, std::move(button)) { }

Keybind::Keybind(const char * button)
: Keybind(std::string(button)) { }

Keybind::Keybind(EPointerButtons button)
: Keybind(M_NONE, button) { }

const std::string& Keybind::getButton() const {
	return button;
}

EKeyModifiers Keybind::getModifiers() const {
	return mods;
}

bool Keybind::looseMatch(EKeyModifiers m, EPointerButtons btn) const {
	char match = '\x00';
	switch (btn) {
		case P_MPRIMARY:
			match = 'L';
			break;

		case P_MSECONDARY:
			match = 'R';
			break;

		case P_MMIDDLE:
			match = 'M';
			break;

		case P_MFOURTH:
			match = '4';
			break;

		case P_MFIFTH:
			match = '5';
			break;

		default:
			break;
	}

	return (m & mods) == mods && button.size() == 6 && button[0] == match;
}

bool Keybind::looseMatch(EKeyModifiers m, const char * key) const {
	return (m & mods) == mods && key == button;
}

bool Keybind::looseMatch(EKeyModifiers m, const std::string& key) const {
	return (m & mods) == mods && key == button;
}

// always strict checking
bool Keybind::operator ==(const Keybind& kb) const {
	return mods == kb.mods && button == kb.button;
}

bool Keybind::operator  <(const Keybind& kb) const {
	if (button < kb.button) {
		return true;
	}

	if (button > kb.button) {
		return false;
	}

	return mods < kb.mods;
}


ImAction::Event::Event(EActionTriggers a)
: activationType(a),
  rejected(false) { }

EActionTriggers ImAction::Event::getActivationType() const {
	return activationType;
}

void ImAction::Event::reject() {
	rejected = true;
}

ImAction::ImAction(InputAdapter& adapter, std::string name, u8 trg,
	std::function<void(ImAction::Event&, const InputInfo&)> cb)
: name(std::move(name)),
  cb(std::move(cb)),
  adapter(adapter),
  enabled(true),
  bindingsChanged(false),
  trg((EActionTriggers)trg) {
  	if (trg & T_ONHOLD) {
  		// ONHOLD implies ONPRESS
  		this->trg = (EActionTriggers)(trg | T_ONPRESS);
  	}

	adapter.add(this);
}

ImAction::ImAction(InputAdapter& adapter, std::string name,
	std::function<void(ImAction::Event&, const InputInfo&)> cb)
: ImAction(adapter, std::move(name), T_ONPRESS, std::move(cb)) { }

ImAction::~ImAction() {
	adapter.del(this);
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

bool ImAction::haveBindingsChanged() const {
	return bindingsChanged;
}

template<typename T>
const Keybind * ImAction::getMatch(EKeyModifiers m, const T key) const {
	for (const Keybind& k : bindings) {
		if (k.looseMatch(m, key)) {
			return &k;
		}
	}

	return nullptr;
}

template const Keybind * ImAction::getMatch<const char *>(EKeyModifiers m, const char *) const;

const Keybind * ImAction::getMatch(EKeyModifiers m, const InputInfo& ii) const {
	for (const Keybind& k : bindings) {
		if (k.looseMatch(m, ii.getButtons())) {
			return &k;
		}
	}

	return nullptr;
}

std::vector<Keybind>& ImAction::getBindings() {
	return bindings;
}

const std::vector<Keybind>& ImAction::getBindings() const {
	return bindings;
}

void ImAction::addKeybind(Keybind kb) {
	bindings.emplace_back(std::move(kb));
	std::sort(bindings.begin(), bindings.end(), [] (const auto& a, const auto& b) {
		return !(a < b);
	});

	bindingsChanged = true;
}

void ImAction::setDefaultKeybind(Keybind kb) {
	if (bindings.empty()) {
		bindings.emplace_back(std::move(kb));
	}
}

void ImAction::setDefaultKeybinds(std::vector<Keybind> kbs) {
	if (bindings.empty()) {
		bindings = std::move(kbs);
	}
}

void ImAction::setEnabled(bool s) {
	enabled = s;
}

void ImAction::setCb(std::function<void(ImAction::Event& e, const InputInfo&)> newCb) {
	cb = std::move(newCb);
}

void ImAction::clearBindingsChanged() {
	bindingsChanged = false;
}


bool ImAction::operator()(EActionTriggers activationType, const InputInfo& ii) {
	if (getTriggers() & activationType) {
		ImAction::Event e(activationType);
		cb(e, ii);
		return !e.rejected;
	}

	return false;
}

bool ImAction::operator ==(const ImAction& rhs) const {
	return name == rhs.name;
}


InputInfo::Pointer::Pointer()
: lastX(0),
  lastY(0),
  x(0),
  y(0),
  btns(P_NONE),
  type(InputInfo::Pointer::MOUSE),
  active(false) { }

int InputInfo::Pointer::getX() const { return x; }
int InputInfo::Pointer::getY() const { return y; }
int InputInfo::Pointer::getDx() const { return x - lastX; }
int InputInfo::Pointer::getDy() const { return y - lastY; }
int InputInfo::Pointer::getLastX() const { return lastX; }
int InputInfo::Pointer::getLastY() const { return lastY; }
EPointerButtons InputInfo::Pointer::getButtons() const { return btns; }
bool InputInfo::Pointer::isActive() const { return active; }

void InputInfo::Pointer::set(int nx, int ny, EPointerButtons nbtns, EType ntype) {
	set(nx, ny, ntype);
	btns = nbtns;
}

void InputInfo::Pointer::set(int nx, int ny, EType ntype) {
	if (active) {
		lastX = x;
		lastY = y;
	} else {
		lastX = nx;
		lastY = ny;
	}

	x = nx;
	y = ny;
	type = ntype;

	active = true;
}

void InputInfo::Pointer::set(EPointerButtons nbtns, EType ntype) {
	btns = nbtns;
	type = ntype;

	active = true;
}

void InputInfo::Pointer::setActive(bool s) {
	active = s;
}


InputInfo::InputInfo()
: wheelDx(0.0),
  wheelDy(0.0),
  updatedPointer(&pointers[0]),
  currentModifiers(M_NONE),
  ptrListOutdated(true) { }

EKeyModifiers InputInfo::getModifiers() const {
	return currentModifiers;
}

double InputInfo::getWheelDx() const {
	return wheelDx;
}

double InputInfo::getWheelDy() const {
	return wheelDy;
}

int InputInfo::getX() const { return updatedPointer->getX(); }
int InputInfo::getY() const { return updatedPointer->getY(); }
int InputInfo::getDx() const { return updatedPointer->getDx(); }
int InputInfo::getDy() const { return updatedPointer->getDy(); }
int InputInfo::getLastX() const { return updatedPointer->getLastX(); }
int InputInfo::getLastY() const { return updatedPointer->getLastY(); }
EPointerButtons InputInfo::getButtons() const { return updatedPointer->getButtons(); }

const std::vector<const InputInfo::Pointer*>& InputInfo::getActivePointers() const {
	if (ptrListOutdated) {
		activePointers.clear();

		for (const InputInfo::Pointer& p : pointers) {
			if (p.isActive()) {
				activePointers.push_back(&p);
			}
		}

		ptrListOutdated = false;
	}

	return activePointers;
}

const InputInfo::Pointer& InputInfo::getPointer(int id) const {
	return pointers[id % pointers.size()];
}

InputInfo::Pointer& InputInfo::getPointer(int id) {
	ptrListOutdated = true;
	InputInfo::Pointer * p = &pointers[id % pointers.size()];
	updatedPointer = p;
	return *p;
}

void InputInfo::setModifiers(EKeyModifiers m) {
	currentModifiers = m;
}

void InputInfo::setWheel(double dx, double dy) {
	wheelDx = dx;
	wheelDy = dy;
}

InputStorage::InputStorage() {
	std::printf("[InputStorage] TODO loading\n");
}

InputStorage::~InputStorage() {
	std::printf("[~InputStorage] TODO saving\n");
}

void InputStorage::popStoredKeybinds(const std::string& name, std::function<void(Keybind)> cb) {
	auto its = savedBindings.equal_range(name);

	for (auto i = its.first; i != its.second; ++i) {
		cb(std::move(i->second));
	}

	savedBindings.erase(its.first, its.second);
}

void InputStorage::storeKeybinds(const std::string& name, std::vector<Keybind>& v) {
	auto hint = savedBindings.upper_bound(name);

	for (auto it = v.begin(); it != v.end(); ++it) {
		savedBindings.emplace_hint(hint, name, std::move(*it));
	}

	v.clear();
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
	if (actions.size() > 0) {
		std::printf("[~InputAdapter] %lu actions still registered on adapter ", actions.size());
		std::printf("%s!!\n", getFullContext().c_str()); // could segfault if any parent is deleted before this
	}
}

const std::string& InputAdapter::getContext() const {
	return context;
}

std::string InputAdapter::getFullContext() const {
	return parentAdapter ? parentAdapter->getFullContext() + '/' + context : context;
}

InputAdapter& InputAdapter::mkAdapter(std::string childContext, int childPriority) {
	auto it = std::find_if(linkedAdapters.begin(), linkedAdapters.end(), [&childContext] (const InputAdapter& a) {
		return a.getContext() == childContext;
	});

	if (it != linkedAdapters.end()) {
		return *it;
	}

	it = std::upper_bound(linkedAdapters.begin(), linkedAdapters.end(), childPriority, [] (const int a, const InputAdapter& b) {
		return b.priority > a;
	});

	if (it != linkedAdapters.end()) {
		it = linkedAdapters.emplace(it, this, storage, std::move(childContext), childPriority);
	} else {
		it = linkedAdapters.emplace(linkedAdapters.end(), this, storage, std::move(childContext), childPriority);
	}

	return *it;
}

void InputAdapter::tick(const InputInfo& ii) const {
	matchEvent(T_ONHOLD, ii);
}

template<typename T>
bool InputAdapter::matchDown(const T key, const InputInfo& ii) {
	if (!enabled) {
		return false;
	}

	for (auto it = linkedAdapters.begin(); it != linkedAdapters.end(); ++it) {
		InputAdapter& adapter = *it;
		if (adapter.matchDown(key, ii)) {
			return true;
		}
	}

	bool consumed = false;
	for (auto it = actions.begin(); it != actions.end(); it++) {
		if (!(*it)->isEnabled()) {
			continue;
		}

		// bug: if the action's keybind changes, onrelease may not be fired
		if (const Keybind * k = (*it)->getMatch(ii.getModifiers(), key)) {
			if (std::find(activeActions.begin(), activeActions.end(), *it) != activeActions.end()
					|| (((*it)->getTriggers() & T_ONPRESS) && !(**it)(T_ONPRESS, ii))) {
				continue;
			}

			if ((*it)->getTriggers() & (T_ONHOLD | T_ONRELEASE | T_ONMOVE
					| T_ONCANCEL | T_ONWHEEL | T_ONLEAVE)) {
				activeActions.emplace_back(*it);
			}

			consumed = true;
		}
	}

	return consumed;
}

template bool InputAdapter::matchDown<const char *>(const char *, const InputInfo&);
template bool InputAdapter::matchDown<EPointerButtons>(const EPointerButtons, const InputInfo&);

void InputAdapter::matchEvent(EActionTriggers trigger, const InputInfo& ii) const {
	for (const auto& adapter : linkedAdapters) {
		adapter.matchEvent(trigger, ii);
	}

	for (auto it = actions.begin(); it != actions.end(); it++) {
		// actions that don't register ONPRESS always get events like move, wheel, etc, except hold
		if ((!((*it)->getTriggers() & T_ONPRESS) && trigger != T_ONHOLD)
				|| std::find(activeActions.begin(), activeActions.end(), *it) != activeActions.end()) {
			(**it)(trigger, ii);
		}
	}
}

template<typename T>
bool InputAdapter::matchUp(const T releasedKey, EActionTriggers lastTrigger,
		const InputInfo& ii) {
	bool consumed = false;

	for (auto it = linkedAdapters.begin(); it != linkedAdapters.end(); ++it) {
		InputAdapter& adapter = *it;
		consumed |= adapter.matchUp(releasedKey, lastTrigger, ii);
	}

	for (auto it = activeActions.begin(); it != activeActions.end();) {
		if (const Keybind * k = (*it)->getMatch(M_ALL, releasedKey)) {
			if (lastTrigger != T_ONRELEASE || ((*it)->getTriggers() & T_ONPRESS)) {
				(**it)(T_ONRELEASE, ii);
			}

			it = activeActions.erase(it);
			consumed = true;
		} else {
			++it;
		}
	}

	return consumed;
}

template bool InputAdapter::matchUp<const char *>(const char *, EActionTriggers, const InputInfo&);
template bool InputAdapter::matchUp<EPointerButtons>(const EPointerButtons, EActionTriggers, const InputInfo&);

void InputAdapter::releaseAll(const InputInfo& ii) {
	for (auto it = linkedAdapters.begin(); it != linkedAdapters.end(); ++it) {
		it->releaseAll(ii);
	}

	for (auto it = activeActions.begin(); it != activeActions.end();) {
		// adapters that don't listen to onpress haven't received anything yet
		// so no need to send cancel & release
		if ((*it)->getTriggers() & T_ONPRESS) {
			(**it)(T_ONCANCEL, ii);
			(**it)(T_ONRELEASE, ii);
		}

		it = activeActions.erase(it);
	}
}


void InputAdapter::add(ImAction * a) {
	std::string fullName(getFullContext() + '/' + a->getName());

	storage.popStoredKeybinds(fullName, [a] (Keybind kb) {
		a->addKeybind(std::move(kb));
	});

	a->clearBindingsChanged();

	std::printf("[InputAdapter] Registered action: %s\n", fullName.c_str());
	actions.push_back(a);
}

void InputAdapter::del(ImAction * a) {
	if (a->haveBindingsChanged()) {
		std::string fullName(getFullContext() + '/' + a->getName());
		storage.storeKeybinds(fullName, a->getBindings());
	}

	auto it = std::find(actions.begin(), actions.end(), a);
	if (it != actions.end()) {
		actions.erase(it);
	}

	it = std::find(activeActions.begin(), activeActions.end(), a);
	if (it != activeActions.end()) {
		if ((*it)->getTriggers() & T_ONPRESS) {
			//(*it)(T_ONRELEASE, ???);
		}

		activeActions.erase(it);
	}
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



InputManager::InputManager(const char * kbTargetElement, const char * ptrTargetElement)
: InputAdapter(nullptr, *this, "Base"),
  kbTargetElement(kbTargetElement),
  ptrTargetElement(ptrTargetElement),
  lastTrigger(T_ONPRESS) {
	emscripten_set_keydown_callback(kbTargetElement, this, true, InputManager::handleKeyEvent);
	emscripten_set_keyup_callback(kbTargetElement, this, true, InputManager::handleKeyEvent);
	emscripten_set_blur_callback(kbTargetElement, this, true, InputManager::handleFocusEvent);

	emscripten_set_mousemove_callback(ptrTargetElement, this, true, InputManager::handleMouseEvent);
	emscripten_set_mousedown_callback(ptrTargetElement, this, true, InputManager::handleMouseEvent);
	emscripten_set_mouseup_callback(ptrTargetElement, this, true, InputManager::handleMouseEvent);
	emscripten_set_mouseleave_callback(ptrTargetElement, this, true, InputManager::handleMouseEvent);
	emscripten_set_mouseenter_callback(ptrTargetElement, this, true, InputManager::handleMouseEvent);

	emscripten_set_touchstart_callback(ptrTargetElement, this, true, InputManager::handleTouchEvent);
	emscripten_set_touchend_callback(ptrTargetElement, this, true, InputManager::handleTouchEvent);
	emscripten_set_touchmove_callback(ptrTargetElement, this, true, InputManager::handleTouchEvent);
	emscripten_set_touchcancel_callback(ptrTargetElement, this, true, InputManager::handleTouchEvent);

	emscripten_set_wheel_callback(ptrTargetElement, this, true, InputManager::handleWheelEvent);

	std::printf("[InputManager] Initialized. Target elements: keyboard=%s pointer=%s\n",
			kbTargetElement == EMSCRIPTEN_EVENT_TARGET_WINDOW ? "window" : kbTargetElement,
			ptrTargetElement == EMSCRIPTEN_EVENT_TARGET_WINDOW ? "window" : ptrTargetElement);
}

InputManager::~InputManager() {
	emscripten_set_keydown_callback(kbTargetElement, nullptr, true, nullptr);
	emscripten_set_keyup_callback(kbTargetElement, nullptr, true, nullptr);
	emscripten_set_blur_callback(kbTargetElement, nullptr, true, nullptr);

	emscripten_set_mousemove_callback(ptrTargetElement, nullptr, true, nullptr);
	emscripten_set_mousedown_callback(ptrTargetElement, nullptr, true, nullptr);
	emscripten_set_mouseup_callback(ptrTargetElement, nullptr, true, nullptr);
	emscripten_set_mouseleave_callback(ptrTargetElement, nullptr, true, nullptr);
	emscripten_set_mouseenter_callback(ptrTargetElement, nullptr, true, nullptr);

	emscripten_set_touchstart_callback(ptrTargetElement, nullptr, true, nullptr);
	emscripten_set_touchend_callback(ptrTargetElement, nullptr, true, nullptr);
	emscripten_set_touchmove_callback(ptrTargetElement, nullptr, true, nullptr);
	emscripten_set_touchcancel_callback(ptrTargetElement, nullptr, true, nullptr);

	emscripten_set_wheel_callback(ptrTargetElement, nullptr, true, nullptr);
	std::printf("[~InputManager]\n");
}

void InputManager::tick() {
	InputAdapter::tick(*this);
}

bool InputManager::keyDown(const char * key) {
	std::printf("[InputManager] KEYDOWN: mods=%d key=%s\n", InputInfo::getModifiers(), key);

	bool handled = matchDown(key, *this);
	lastTrigger = T_ONPRESS;

	return handled;
}

bool InputManager::keyUp(const char * key) {
	std::printf("[InputManager] KEYUP: mods=%d key=%s\n", InputInfo::getModifiers(), key);

	bool handled = matchUp(key, lastTrigger, *this);
	lastTrigger = T_ONRELEASE;

	return handled;
}

bool InputManager::pointerDown(int id, Ptr::EType t, EPointerButtons changed, EPointerButtons buttons) {
	std::printf("[InputManager] MDOWN: id=%d type=%c mods=%d changes=%d buttons=%d\n",
			id, t == Ptr::MOUSE ? 'M' : 'T', InputInfo::getModifiers(), changed, buttons);

	InputInfo::getPointer(id).set(buttons, t);

	lastTrigger = T_ONPRESS;
	matchDown(changed, *this);

	return false;
}

bool InputManager::pointerUp(int id, Ptr::EType t, EPointerButtons changed, EPointerButtons buttons) {
	std::printf("[InputManager] MUP: id=%d type=%c mods=%d changes=%d buttons=%d\n",
			id, t == Ptr::MOUSE ? 'M' : 'T', InputInfo::getModifiers(), changed, buttons);

	InputInfo::getPointer(id).set(buttons, t);

	matchUp(changed, lastTrigger, *this);

	return false;
}

void InputManager::pointerMove(int id, Ptr::EType t, int x, int y) {
	//std::printf("[InputManager] MMOVE: x=%d y=%d\n", x, y);
	InputInfo::getPointer(id).set(x, y, t);
	matchEvent(T_ONMOVE, *this);
}

void InputManager::pointerCancel(int id, Ptr::EType t) {
	matchEvent(T_ONCANCEL, *this);
}

void InputManager::pointerEnter(int id, Ptr::EType t) {
	InputInfo::getPointer(id).setActive(true);
}

void InputManager::pointerLeave(int id, Ptr::EType t) {
	InputInfo::getPointer(id).setActive(false);
	matchEvent(T_ONLEAVE, *this);
}

bool InputManager::wheel(double dx, double dy, int type) {
	//std::printf("[InputManager] WHEEL: dx=%f dy=%f dz=%f type=%d\n", dx, dy, dz, type);
	InputInfo::setWheel(dx, dy);

	matchEvent(T_ONWHEEL, *this);

	return false;
}

void InputManager::setModifiers(bool ctrl, bool alt, bool shift, bool meta) {
	int m = M_NONE;
	m |= static_cast<int>(ctrl);
	m |= static_cast<int>(alt) << 1;
	m |= static_cast<int>(shift) << 2;
	m |= static_cast<int>(meta) << 3;

	InputInfo::setModifiers(static_cast<EKeyModifiers>(m));
}

void InputManager::lostFocus() {
	std::printf("[InputManager] BLUR\n");
	InputAdapter::releaseAll(*this);
	setModifiers(false, false, false, false);
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

	im->setModifiers(ev->ctrlKey, ev->altKey, ev->shiftKey, ev->metaKey);

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

	im->setModifiers(ev->ctrlKey, ev->altKey, ev->shiftKey, ev->metaKey);

	int changed = ev->button;
	switch (changed) {
		case 1:
			changed = 2;
			break;

		case 2:
			changed = 1;
			break;

		default:
			break;
	}

	changed = 1 << changed;

	switch (type) {
		case EMSCRIPTEN_EVENT_MOUSEMOVE:
			im->pointerMove(0, Ptr::MOUSE, ev->clientX, ev->clientY);
			return false;

		case EMSCRIPTEN_EVENT_MOUSEDOWN:
			return im->pointerDown(0, Ptr::MOUSE,
				static_cast<EPointerButtons>(changed),
				static_cast<EPointerButtons>(ev->buttons));

		case EMSCRIPTEN_EVENT_MOUSEUP:
			return im->pointerUp(0, Ptr::MOUSE,
				static_cast<EPointerButtons>(changed),
				static_cast<EPointerButtons>(ev->buttons));

		case EMSCRIPTEN_EVENT_MOUSELEAVE:
			im->pointerLeave(0, Ptr::MOUSE);
			return false;

		case EMSCRIPTEN_EVENT_MOUSEENTER:
			im->pointerEnter(0, Ptr::MOUSE);
			return false;
	}

	return false;
}

int InputManager::handleTouchEvent(int type, const EmscriptenTouchEvent * ev, void * data) {
	InputManager * im = static_cast<InputManager *>(data);

	im->setModifiers(ev->ctrlKey, ev->altKey, ev->shiftKey, ev->metaKey);

	for (int i = 0; i < ev->numTouches; i++) {
		const EmscriptenTouchPoint& tp = ev->touches[i];

		if (!tp.isChanged) {
			continue;
		}

		switch (type) {
			case EMSCRIPTEN_EVENT_TOUCHSTART:
				im->pointerMove(tp.identifier, Ptr::TOUCH, tp.clientX, tp.clientY);
				im->pointerEnter(tp.identifier, Ptr::TOUCH);
				im->pointerDown(tp.identifier, Ptr::TOUCH, P_MPRIMARY, P_MPRIMARY);
				break;

			case EMSCRIPTEN_EVENT_TOUCHEND:
				im->pointerUp(tp.identifier, Ptr::TOUCH, P_MPRIMARY, P_NONE);
				im->pointerLeave(tp.identifier, Ptr::TOUCH);
				break;

			case EMSCRIPTEN_EVENT_TOUCHMOVE:
				im->pointerMove(tp.identifier, Ptr::TOUCH, tp.clientX, tp.clientY);
				break;

			case EMSCRIPTEN_EVENT_TOUCHCANCEL:
				im->pointerCancel(tp.identifier, Ptr::TOUCH);
				break;
		}
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

	im->setModifiers(ev->mouse.ctrlKey, ev->mouse.altKey, ev->mouse.shiftKey, ev->mouse.metaKey);

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
