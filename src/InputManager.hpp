#pragma once

#include <utility>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <vector>
#include <array>

#include "explints.hpp"

// order of the binding matters: pressing C + CTRL should not trigger CTRL + C bindings.
// EXCEPT: keyboard + mouse bindings. mouse input is appended after keyboard.
// example: pressing MCLICK1 + CTRL would only activate CTRL + MCLICK1 bindings.

// order of release also matters: releasing CTRL before C should not trigger the action,
// except if activation type is not on release OR is listening on both press and release.

// order of activation on multiple actions for the same binding is by alphabetical order,
// and stops when action was active && returns true on call -- not implemented

// mixed (keyboard + mouse) bindings are strictly matched CTRL + MCLICK2 + MCLICK1
// does not trigger CTRL + MCLICK1 bindings, unless activation type is on release/move, and MCLICK2
// would be released before MCLICK1

// simple (keyboard OR mouse) bindings are more loosely matched: F1 + MCLICK1 would
// trigger an action bound as F1 and another bound with MCLICK1, unless
// there is already a mixed binding listening to F1 + MCLICK1.

// guarantee balance of PRESS/RELEASE calls, if listening on both

// examples:

// consider two bindings only listening on RELEASE:
// CTRL + C + F1
// CTRL + C
// pressing CTRL + C + F1 and releasing F1 should trigger the first binding,
// but releasing C after that should not trigger the CTRL + C binding.
// don't apply this to bindings listening on both PRESS, and RELEASE.

// handling of these separate bindings listening on PRESS:
// W
// A
// S
// W + A + S
// holding W then holding A would first try to activate W + A bindings, and if
// it fails, the A binding should be activated. holding S would activate W + A + S,
// and wouldn't activate S.

// types of activation:
// PRESS
// HOLD (frequent calls)
// RELEASE

class InputManager;
class InputAdapter;
class InputInfo;
class EmscriptenKeyboardEvent;
class EmscriptenFocusEvent;
class EmscriptenMouseEvent;
class EmscriptenWheelEvent;

enum EPointerEvents {
	P_NONE       = 0,

	P_MPRIMARY   = 1,
	P_MSECONDARY = 2,
	P_MMIDDLE    = 4,
	P_MFOURTH    = 8,
	P_MFIFTH     = 16,

	P_MWHEEL = 128,

	P_TOUCH = 256
};

enum EActionTriggers : u8 {
	T_ONPRESS   = 1,
	T_ONHOLD    = 2,
	T_ONRELEASE = 4,
	T_ONMOVE    = 8 // mouse move or touch move
};

class Keybind {
protected:
	// TODO: deduplicate across multiple keybinds?
	std::vector<std::string> kbKeys;
	EPointerEvents mButtons;

public:
	Keybind(std::vector<std::string> = {}, EPointerEvents = P_NONE);

	const std::vector<std::string>& getKeyboardKeys() const;
	EPointerEvents getPointerEvents() const;
	bool hasKbKeys() const;
	bool isMixed() const;

	// loose or strict match depending on parameter's .isMixed()
	bool canActivate(const Keybind&) const;
	bool contains(const char * key) const;

	// always strict checking
	bool operator ==(const Keybind&) const;
	bool operator  <(const Keybind&) const;
};

class ImAction {
	const std::string name;
	std::function<void(const Keybind&, EActionTriggers activationType, const InputInfo& ii)> cb;
	InputAdapter& adapter;
	bool enabled;
	bool bindable; // do i need this?
	EActionTriggers trg;

public:
	ImAction(std::string, u8 triggers, InputAdapter&,
		std::function<void(const Keybind&, EActionTriggers activationType, const InputInfo& ii)>);
	~ImAction();

	const std::string& getName() const;
	EActionTriggers getTriggers() const;
	bool isEnabled() const;

	void setEnabled(bool);
	void setCb(std::function<void(const Keybind&, EActionTriggers activationType, const InputInfo& ii)>);

	bool operator ()(const Keybind&, EActionTriggers activationType, const InputInfo& ii);
	bool operator ==(const ImAction&) const;
};

class InputInfo { // passed to action callbacks
	double wheelDx;
	double wheelDy;
	int lastMouseX;
	int lastMouseY;
	int mouseX;
	int mouseY;

public:
	InputInfo();

	double getWheelDx() const;
	double getWheelDy() const;
	int getLastMouseX() const;
	int getLastMouseY() const;
	int getMouseX() const;
	int getMouseY() const;

	void setWheel(double, double);
	void setMouse(int, int);
	void updateLastMouse();
};

class InputStorage {
	// for actions that haven't been registered yet, and have custom keybinds
	// can include restores for sub-actions, example:
	// Base/Tool/Cursor/Draw => P_MPRIMARY
	// Last section of the string is always the action name
	std::map<std::string, Keybind> savedBindings;

public:
	InputStorage(); // load from localStorage
	~InputStorage(); // save to localStorage

	std::optional<Keybind> popStoredKeybind(const std::string&);
	void storeKeybind(std::string, Keybind);
};

class InputAdapter {
	InputAdapter * const parentAdapter;
	InputStorage& storage;
	const std::string context;
	const int priority;
	bool enabled;

	std::map<Keybind, std::weak_ptr<ImAction>> bindings;
	// Holds bindings which were sent a T_ONPRESS event (and may receive T_ONHOLD & T_ONRELEASE, if specified)
	std::vector<decltype(bindings)::const_reverse_iterator> activeBindings;

	std::set<InputAdapter> linkedAdapters;

public:
	InputAdapter(InputAdapter * parent, InputStorage& is, std::string context, int priority = 0);
	InputAdapter(const InputAdapter&) = delete;
	~InputAdapter();

	std::string getFullContext() const; // very slow
	InputAdapter& mkAdapter(std::string context, int priority = 0);

	// calls HOLD for T_ONHOLD-registered and active (held) keybinds
	void tick(const InputInfo&) const;
	bool matchDown(const Keybind& currentKeys, const char * pressedKey, EActionTriggers lastTrigger, const InputInfo&);
	bool matchUpdate(EPointerEvents EActionTriggers lastTrigger, const InputInfo&);
	bool matchUp(const Keybind& currentKeys, const char * releasedKey, EActionTriggers lastTrigger, const InputInfo&);

	std::shared_ptr<ImAction> add(std::string name, Keybind defaultKb, u8 triggers,
			std::function<void(const Keybind&, EActionTriggers, const InputInfo&)>);

	void del(const std::string&);

	bool setKeybinding(const std::shared_ptr<ImAction>&, Keybind);

	bool operator <(const InputAdapter&) const;
};

// Order of destruction of derived classes will guarantee InputStorage is still
// valid when InputAdapter gets destroyed, allowing saving
class InputManager : Keybind, InputInfo, InputStorage, public InputAdapter {
	const char * targetElement;
	long tickIntervalId;

	// only holds either T_ONPRESS or T_ONRELEASE
	EActionTriggers lastTrigger;

public:
	InputManager(const char * targetElem);
	~InputManager();

	void printHeldKeys() const;

	void tick();

	bool keyDown(const char * key);
	bool keyUp(const char * key);
	bool mouseDown(int changed, int buttons);
	bool mouseUp(int changed, int buttons);
	bool mouseMove(int x, int y);
	bool wheel(double dx, double dy, int type);

	void lostFocus();

private:
	static int handleKeyEvent(int type, const EmscriptenKeyboardEvent * ev, void * data);
	static int handleMouseEvent(int type, const EmscriptenMouseEvent * ev, void * data);
	static int handleFocusEvent(int type, const EmscriptenFocusEvent * ev, void * data);
	static int handleWheelEvent(int type, const EmscriptenWheelEvent * ev, void * data);
};
