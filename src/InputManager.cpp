#include "InputManager.hpp"

#include <algorithm>
#include <numeric>
#include <memory>
#include <cctype>
#include <cstdio>
#include <cstring>

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

	return (m & mods) == mods && (button == ANY_PTR_BTN || (button.size() == 6 && button[0] == match));
}

bool Keybind::looseMatch(EKeyModifiers m, const char * key) const {
	return (m & mods) == mods && (button == ANY_KB_BTN || key == button);
}

bool Keybind::looseMatch(EKeyModifiers m, const std::string& key) const {
	return (m & mods) == mods && (button == ANY_KB_BTN || key == button);
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

ImAction::ImAction(InputAdapter& adapter, std::string name, u32 trg,
	std::function<void(ImAction::Event&, const InputInfo&)> cb)
: name(std::move(name)),
  cb(std::move(cb)),
  adapter(adapter),
  enabled(true),
  bindingsChanged(false),
  trg((EActionTriggers)trg) {
	if (trg & T_ONHOLD) {
		// ONHOLD implies ONPRESS, either remove this or add a short delay to receive ONHOLD events
		this->trg = (EActionTriggers)(trg | T_ONPRESS);
	}

	adapter.add(this);
}

ImAction::ImAction(InputAdapter& adapter, std::string name,
	std::function<void(ImAction::Event&, const InputInfo&)> cb)
: ImAction(adapter, std::move(name), T_ONPRESS, std::move(cb)) { }

ImAction::~ImAction() {
	adapter.del(this);
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
  present(false) { }

int InputInfo::Pointer::getX() const { return x; }
int InputInfo::Pointer::getY() const { return y; }
int InputInfo::Pointer::getDx() const { return x - lastX; }
int InputInfo::Pointer::getDy() const { return y - lastY; }
int InputInfo::Pointer::getLastX() const { return lastX; }
int InputInfo::Pointer::getLastY() const { return lastY; }
EPointerButtons InputInfo::Pointer::getButtons() const { return btns; }
InputInfo::Pointer::EType InputInfo::Pointer::getType() const { return type; }
bool InputInfo::Pointer::isPresent() const { return present; }
bool InputInfo::Pointer::isActive() const { return btns != P_NONE; }

void InputInfo::Pointer::finishMoving() {
	lastX = x;
	lastY = y;
}

void InputInfo::Pointer::set(int nx, int ny, EPointerButtons nbtns, EType ntype) {
	set(nx, ny, ntype);
	btns = nbtns;
}

void InputInfo::Pointer::set(int nx, int ny, EType ntype) {
	if (present) {
		lastX = x;
		lastY = y;
	} else {
		lastX = nx;
		lastY = ny;
	}

	x = nx;
	y = ny;
	type = ntype;

	present = true;
}

void InputInfo::Pointer::set(EPointerButtons nbtns, EType ntype) {
	btns = nbtns;
	type = ntype;

	present |= btns != P_NONE;
}

void InputInfo::Pointer::setPresent(bool s) {
	present = s;
	if (!s) {
		btns = P_NONE;
	}
}


InputInfo::InputInfo()
: timestamp(0.0),
  wheelDx(0.0),
  wheelDy(0.0),
  updatedPointer(&pointers[0]),
  currentModifiers(M_NONE),
  ptrListOutdated(true) { }

EKeyModifiers InputInfo::getModifiers() const {
	return currentModifiers;
}

double InputInfo::getTimestamp() const {
	return timestamp;
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

float InputInfo::getMidX() const {
	const auto& p = getPointers();

	if (p.size() == 0) {
		// get pos from last updated pointer
		return getX();
	}

	return std::accumulate(p.cbegin(), p.cend(), 0.f, [] (float a, const Pointer* b) {
		return a + b->getX();
	}) / static_cast<float>(p.size());
}

float InputInfo::getMidY() const {
	const auto& p = getPointers();

	if (p.size() == 0) {
		return getY();
	}

	return std::accumulate(p.cbegin(), p.cend(), 0.f, [] (float a, const Pointer* b) {
		return a + b->getY();
	}) / static_cast<float>(p.size());
}

float InputInfo::getMidDx() const {
	auto midX = getMidX();
	auto lastMidX = getLastMidX();
	return midX - lastMidX;
}

float InputInfo::getMidDy() const {
	auto midY = getMidY();
	auto lastMidY = getLastMidY();
	return midY - lastMidY;
}

float InputInfo::getLastMidX() const {
	const auto& p = getPointers();

	if (p.size() == 0) {
		return getX();
	}

	return std::accumulate(p.cbegin(), p.cend(), 0.f, [] (float a, const Pointer* b) {
		return a + b->getLastX();
	}) / static_cast<float>(p.size());
}

float InputInfo::getLastMidY() const {
	const auto& p = getPointers();

	if (p.size() == 0) {
		return getY();
	}

	return std::accumulate(p.cbegin(), p.cend(), 0.f, [] (float a, const Pointer* b) {
		return a + b->getLastY();
	}) / static_cast<float>(p.size());
}

EPointerButtons InputInfo::getButtons() const { return updatedPointer->getButtons(); }
InputInfo::Pointer::EType InputInfo::getType() const { return updatedPointer->getType(); }

int InputInfo::getNumPointers() const {
	return getPointers().size();
}

int InputInfo::getNumActivePointers() const {
	int num = 0;
	for (InputInfo::Pointer& p : pointers) {
		num += p.isActive();
	}

	return num;
}

const std::vector<InputInfo::Pointer*>& InputInfo::getPointers() const {
	if (ptrListOutdated) {
		pointersPresent.clear();

		for (InputInfo::Pointer& p : pointers) {
			if (p.isActive()) {
				pointersPresent.insert(pointersPresent.begin(), &p);
			} else if (p.isPresent()) {
				pointersPresent.push_back(&p);
			}
		}

		ptrListOutdated = false;
	}

	return pointersPresent;
}

const InputInfo::Pointer& InputInfo::getPointer(int id) const {
	return id < 0 ? *updatedPointer : pointers[id % pointers.size()];
}

InputInfo::Pointer& InputInfo::getPointer(int id) {
	ptrListOutdated = true;
	InputInfo::Pointer * p = id < 0 ? updatedPointer : &pointers[id % pointers.size()];
	updatedPointer = p;
	return *p;
}

void InputInfo::finishMoving() {
	for (InputInfo::Pointer* p : getPointers()) {
		p->finishMoving();
	}
}

void InputInfo::setModifiers(EKeyModifiers m) {
	currentModifiers = m;
}

void InputInfo::setWheel(double dx, double dy) {
	wheelDx = dx;
	wheelDy = dy;
}

void InputInfo::setTimestamp(double ts) {
	timestamp = ts;
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
: im(parent == nullptr ? *static_cast<const InputManager*>(this) : parent->getInputManager()),
  parentAdapter(parent),
  storage(is),
  context(std::move(context)),
  priority(priority),
  enabled(true) {
#ifdef DEBUG
	std::printf("[InputAdapter] Registered %s, prio %d\n", getFullContext().c_str(), priority);
#endif
}

InputAdapter::~InputAdapter() {
#ifdef DEBUG
	std::printf("[~InputAdapter] Del %s\n", context.c_str());
	if (actions.size() > 0) {
		std::printf("[~InputAdapter] %lu actions still registered on adapter ", actions.size());
		std::printf("%s!!\n", getFullContext().c_str()); // could segfault if any parent is deleted before this
	}
#endif
}

const InputManager& InputAdapter::getInputManager() const {
	return im;
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
			constexpr bool eventIsPointer = std::is_same_v<T, EPointerButtons>;
			bool isActive = std::find(activeActions.begin(), activeActions.end(), *it) != activeActions.end();
			EActionTriggers trgs = (*it)->getTriggers();

			// skip sending multiple T_ONPRESS on this action if it's keyboard activated
			// for pointer activated actions multiple presses can be sent because of touch events
			if (isActive && !eventIsPointer) {
				continue;
			}

			// skip activating if event was rejected, don't send onpress here if action requests all events,
			// InputAdapter::matchEvent will send it instead
			if ((trgs & T_ONPRESS) && !(trgs & T_OPT_ALWAYS) && !(**it)(T_ONPRESS, ii)) {
				continue;
			}

			if (((*it)->getTriggers() & (T_ONHOLD | T_ONRELEASE | T_ONMOVE
					| T_ONCANCEL | T_ONWHEEL | T_ONLEAVE)) && !isActive) {
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
	// these events are only sent by this function if T_OPT_ALWAYS is active
	bool isSpecialTrg = trigger & (T_ONPRESS | T_ONRELEASE);

	for (auto it = actions.begin(); it != actions.end(); it++) {
		// actions that don't register ONPRESS always get events like move, wheel, etc, except hold
		EActionTriggers trgs = (*it)->getTriggers();
		if (isSpecialTrg && !(trgs & T_OPT_ALWAYS)) {
			continue;
		}

		if ( (!(trgs & T_ONPRESS) && trigger != T_ONHOLD)
				|| (trgs & T_OPT_ALWAYS) // if the action asks for it, always send all events
				|| trigger == T_ONENTER // ONENTER is always sent
				|| std::find(activeActions.begin(), activeActions.end(), *it) != activeActions.end()) {
			(**it)(trigger, ii);
		}
	}

	// hackish solution to give more priority to nearer actions (for cursor updating...)
	for (const auto& adapter : linkedAdapters) {
		adapter.matchEvent(trigger, ii);
	}
}

template<typename T>
bool InputAdapter::matchUp(const T releasedKey, EActionTriggers lastTrigger,
		const InputInfo& ii) {
	constexpr bool eventIsPointer = std::is_same_v<T, EPointerButtons>;
	bool consumed = false;

	for (auto it = linkedAdapters.begin(); it != linkedAdapters.end(); ++it) {
		InputAdapter& adapter = *it;
		consumed |= adapter.matchUp(releasedKey, lastTrigger, ii);
	}

	for (auto it = activeActions.begin(); it != activeActions.end();) {
		EActionTriggers trgs = (*it)->getTriggers();
		if (const Keybind * k = (*it)->getMatch(M_ALL, releasedKey)) {
			if (!(trgs & T_OPT_ALWAYS) && (lastTrigger != T_ONRELEASE || (trgs & T_ONPRESS))) {
				(**it)(T_ONRELEASE, ii);
			}

			consumed = true;
			if (!eventIsPointer || ii.getNumActivePointers() == 0) {
				it = activeActions.erase(it);
				continue;
			}
		}

		++it;
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

#ifdef DEBUG
	std::printf("[InputAdapter] Registered action: %s\n", fullName.c_str());
#endif
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



InputManager::InputManager(const char * ptrActionAreaTargetElement)
: InputAdapter(nullptr, *this, "Base"),
  kbTargetElement(EMSCRIPTEN_EVENT_TARGET_DOCUMENT),
  ptrTargetElement(EMSCRIPTEN_EVENT_TARGET_DOCUMENT),
  ptrActionAreaTargetElement(ptrActionAreaTargetElement),
  disabled(false),
  lastTrigger(T_ONPRESS) {
	emscripten_set_keydown_callback(kbTargetElement, this, false, InputManager::handleKeyEvent);
	emscripten_set_keyup_callback(kbTargetElement, this, false, InputManager::handleKeyEvent);
	emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, InputManager::handleFocusEvent);
	emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, InputManager::handleFocusEvent);

	emscripten_set_mousemove_callback(ptrTargetElement, this, false, InputManager::handleMouseEvent);
	emscripten_set_mousedown_callback(ptrTargetElement, this, false, InputManager::handleMouseEvent);
	emscripten_set_mouseup_callback(ptrTargetElement, this, false, InputManager::handleMouseEvent);
	emscripten_set_mouseleave_callback(ptrActionAreaTargetElement, this, true, InputManager::handleMouseEvent);
	emscripten_set_mouseenter_callback(ptrActionAreaTargetElement, this, true, InputManager::handleMouseEvent);

	emscripten_set_touchstart_callback(ptrTargetElement, this, false, InputManager::handleTouchEvent);
	emscripten_set_touchend_callback(ptrTargetElement, this, false, InputManager::handleTouchEvent);
	emscripten_set_touchmove_callback(ptrTargetElement, this, false, InputManager::handleTouchEvent);
	emscripten_set_touchcancel_callback(ptrTargetElement, this, false, InputManager::handleTouchEvent);

	emscripten_set_wheel_callback(ptrTargetElement, this, false, InputManager::handleWheelEvent);

	std::puts("[InputManager] Initialized.");
}

InputManager::~InputManager() {
	emscripten_set_keydown_callback(kbTargetElement, nullptr, false, nullptr);
	emscripten_set_keyup_callback(kbTargetElement, nullptr, false, nullptr);
	emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, nullptr);
	emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, nullptr);

	emscripten_set_mousemove_callback(ptrTargetElement, nullptr, false, nullptr);
	emscripten_set_mousedown_callback(ptrTargetElement, nullptr, false, nullptr);
	emscripten_set_mouseup_callback(ptrTargetElement, nullptr, false, nullptr);
	emscripten_set_mouseleave_callback(ptrActionAreaTargetElement, nullptr, true, nullptr);
	emscripten_set_mouseenter_callback(ptrActionAreaTargetElement, nullptr, true, nullptr);

	emscripten_set_touchstart_callback(ptrTargetElement, nullptr, false, nullptr);
	emscripten_set_touchend_callback(ptrTargetElement, nullptr, false, nullptr);
	emscripten_set_touchmove_callback(ptrTargetElement, nullptr, false, nullptr);
	emscripten_set_touchcancel_callback(ptrTargetElement, nullptr, false, nullptr);

	emscripten_set_wheel_callback(ptrTargetElement, nullptr, false, nullptr);
	std::printf("[~InputManager]\n");
}


const InputInfo& InputManager::getLastInputInfo() const {
	return *this;
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

bool InputManager::pointerDown(int id, Ptr::EType t, EPointerButtons changed, EPointerButtons buttons, bool fireEvent) {
//	std::printf("[InputManager] MDOWN: id=%d type=%c mods=%d changes=%d buttons=%d\n",
//			id, t == Ptr::MOUSE ? 'M' : 'T', InputInfo::getModifiers(), changed, buttons);

	InputInfo::Pointer& pointer = InputInfo::getPointer(id);
	pointer.finishMoving();
	pointer.set(buttons, t);

	lastTrigger = T_ONPRESS;
	if (fireEvent) {
		matchEvent(T_ONPRESS, *this);
		matchDown(changed, *this);
	}

	return false;
}

bool InputManager::pointerUp(int id, Ptr::EType t, EPointerButtons changed, EPointerButtons buttons, bool fireEvent) {
//	std::printf("[InputManager] MUP: id=%d type=%c mods=%d changes=%d buttons=%d\n",
//			id, t == Ptr::MOUSE ? 'M' : 'T', InputInfo::getModifiers(), changed, buttons);

	InputInfo::Pointer& pointer = InputInfo::getPointer(id);
	pointer.finishMoving();
	pointer.set(buttons, t);

	if (fireEvent) {
		matchEvent(T_ONRELEASE, *this);
		matchUp(changed, lastTrigger, *this);
	}

	return false;
}

void InputManager::pointerMove(int id, Ptr::EType t, int x, int y, bool fireEvent) {
	//std::printf("[InputManager] MMOVE: x=%d y=%d\n", x, y);
	InputInfo::getPointer(id).set(x, y, t);
	if (fireEvent) {
		matchEvent(T_ONMOVE, *this);
	}
}

void InputManager::pointerCancel(int id, Ptr::EType t, bool fireEvent) {
	if (fireEvent) {
		matchEvent(T_ONCANCEL, *this);
	}
}

void InputManager::pointerEnter(int id, Ptr::EType t, bool fireEvent) {
	InputInfo::getPointer(id).setPresent(true);
	if (fireEvent) {
		matchEvent(T_ONENTER, *this);
	}
}

void InputManager::pointerLeave(int id, Ptr::EType t, bool fireEvent) {
	InputInfo::Pointer& pointer = InputInfo::getPointer(id);
	if (!pointer.isActive()) { // pointer can leave screen but still have buttons pressed
		pointer.setPresent(false);
	}

	if (fireEvent) {
		matchEvent(T_ONLEAVE, *this);
	}
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

void InputManager::setTimestamp(double ts) {
	InputInfo::setTimestamp(ts);
}

void InputManager::setDisabled(bool state) {
	//std::printf("dis: %d\n", state);
	disabled = state;
	if (disabled) {
		InputAdapter::releaseAll(*this);
		setModifiers(false, false, false, false);
	}
}

void InputManager::lostFocus() {
	std::printf("[InputManager] BLUR\n");
	InputAdapter::releaseAll(*this);
	setModifiers(false, false, false, false);
}

int InputManager::handleKeyEvent(int type, const EmscriptenKeyboardEvent * ev, void * data) {
	InputManager * im = static_cast<InputManager *>(data);

	if (im->disabled) {
		return false;
	}

	im->setTimestamp(ev->timestamp);
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

	im->setTimestamp(ev->timestamp);
	im->setModifiers(ev->ctrlKey, ev->altKey, ev->shiftKey, ev->metaKey);

	//std::printf("%d, %d\n", ev->targetX, ev->targetY);

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
			im->pointerMove(0, Ptr::MOUSE, ev->clientX, ev->clientY, false);
			return im->pointerDown(0, Ptr::MOUSE,
				static_cast<EPointerButtons>(changed),
				static_cast<EPointerButtons>(ev->buttons));

		case EMSCRIPTEN_EVENT_MOUSEUP:
			im->pointerMove(0, Ptr::MOUSE, ev->clientX, ev->clientY, false);
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

	im->setTimestamp(ev->timestamp);
	im->finishMoving();
	im->setModifiers(ev->ctrlKey, ev->altKey, ev->shiftKey, ev->metaKey);
	bool cancel = true;
	u32 eventsToTrigger = 0;

	for (int i = 0; i < ev->numTouches; i++) {
		const EmscriptenTouchPoint& tp = ev->touches[i];

		if (!tp.isChanged) {
			continue;
		}

		switch (type) {
			case EMSCRIPTEN_EVENT_TOUCHSTART:
				im->pointerMove(tp.identifier, Ptr::TOUCH, tp.clientX, tp.clientY, false);
				im->pointerEnter(tp.identifier, Ptr::TOUCH, false);
				im->pointerDown(tp.identifier, Ptr::TOUCH, P_MPRIMARY, P_MPRIMARY, false);
				eventsToTrigger |= T_ONENTER | T_ONPRESS;
				break;

			case EMSCRIPTEN_EVENT_TOUCHCANCEL:
				im->pointerCancel(tp.identifier, Ptr::TOUCH, false);
				eventsToTrigger |= T_ONCANCEL;
				cancel = false;
				[[fallthrough]];
			case EMSCRIPTEN_EVENT_TOUCHEND:
				im->pointerUp(tp.identifier, Ptr::TOUCH, P_MPRIMARY, P_NONE, false);
				im->pointerLeave(tp.identifier, Ptr::TOUCH, false);
				eventsToTrigger |= T_ONRELEASE | T_ONLEAVE;
				break;

			case EMSCRIPTEN_EVENT_TOUCHMOVE:
				im->pointerMove(tp.identifier, Ptr::TOUCH, tp.clientX, tp.clientY, false);
				eventsToTrigger |= T_ONMOVE;
				break;
		}
	}

	if (eventsToTrigger & T_ONENTER) {
		im->matchEvent(T_ONENTER, *im);
	}

	if (eventsToTrigger & T_ONPRESS) {
		im->matchDown(P_MPRIMARY, *im);
	}

	if (eventsToTrigger & T_ONMOVE) {
		im->matchEvent(T_ONMOVE, *im);
	}

	if (eventsToTrigger & T_ONCANCEL) {
		im->matchEvent(T_ONCANCEL, *im);
	}

	if (eventsToTrigger & T_ONRELEASE) {
		im->matchUp(P_MPRIMARY, im->lastTrigger, *im);
	}

	if (eventsToTrigger & T_ONLEAVE) {
		im->matchEvent(T_ONLEAVE, *im);
	}

	return cancel;
}

int InputManager::handleFocusEvent(int type, const EmscriptenFocusEvent * e, void * data) {
	InputManager * im = static_cast<InputManager *>(data);

	bool isWindow = std::strcmp("#window", e->nodeName) == 0;
	bool isKeyboardCapturingElem = std::strcmp("INPUT", e->nodeName) == 0;
	//std::printf("%s, %s, %d, %d\n", e->nodeName, e->id, isWindow, isKeyboardCapturingElem);

	// avoid dumb blur events
	if (!isWindow && !isKeyboardCapturingElem) {
		return false;
	}

	switch (type) {
		case EMSCRIPTEN_EVENT_FOCUS:
			im->setDisabled(isKeyboardCapturingElem);
			break;

		case EMSCRIPTEN_EVENT_BLUR:
			if (isWindow) {
				im->setDisabled(false);
				im->lostFocus();
			} else {
				im->setDisabled(!isKeyboardCapturingElem);
			}
			break;
	}

	return false;
}

int InputManager::handleWheelEvent(int type, const EmscriptenWheelEvent * ev, void * data) {
	InputManager * im = static_cast<InputManager *>(data);

	im->setTimestamp(ev->mouse.timestamp);
	im->setModifiers(ev->mouse.ctrlKey, ev->mouse.altKey, ev->mouse.shiftKey, ev->mouse.metaKey);
	im->pointerMove(0, Ptr::MOUSE, ev->mouse.clientX, ev->mouse.clientY, false);

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

