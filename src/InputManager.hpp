#pragma once

#include <utility>
#include <functional>
#include <map>
#include <set>
#include <vector>
#include <array>

// order of the binding matters: pressing C + CTRL should not trigger CTRL + C bindings.
// EXCEPT: keyboard + mouse bindings. mouse input is appended after keyboard.
// example: pressing MCLICK1 + CTRL would only activate CTRL + MCLICK1 bindings.

// order of release also matters: releasing CTRL before C should not trigger the action,
// except if activation type is not on release OR is listening on both press and release.

// order of activation on multiple actions for the same binding is by alphabetical order,
// and stops when action was active && returns true on call

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
// FIX: store a boolean specifying to the current key release check if a key was
// released previously or pressed.
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
// RELEASE

enum EKeybindTriggers : u8 {
	K_ONPRESS   = 1,
	K_ONRELEASE = 2
}

class Keybind {
	// TODO: deduplicate across multiple keybinds?
	std::array<std::vector<std::string>, 2> keys;
	EKeybindTriggers trg;

public:
	Keybind(std::array<std::vector<std::string>, 2>, EKeybindTriggers = K_ONPRESS);
	Keybind(EKeybindTriggers = K_ONPRESS);

	EKeybindTriggers getTriggers() const;
	bool isMixed() const;

	// loose or strict match depending on parameter's .isMixed()
	// isPress = current event is a keypress?
	// wasPress = last event was a keypress?
	bool canActivate(const Keybind&, bool isPress, bool wasPress);

	// always strict checking
	bool operator ==(const Keybind&) const;
	bool operator  <(const Keybind&) const;
};

class ImAction {
	const std::string context;
	const std::string name;
	std::function<bool(const Keybind&, bool isPressed)> cb;
	bool enabled;
	bool bindable;

public:
	ImAction(std::string, std::string, std::function<bool(const Keybind&, bool))>);

	const std::string& getContext() const;
	const std::string& getName() const;
	bool isEnabled() const;

	void setEnabled(bool);
	void setCb(std::function<bool(const Keybind&, bool)>);

	bool operator()(const Keybind&, bool isPressed);
};

class ImActionProxy {
	ImAction * a;
	InputManager& im;

public:
	ImActionProxy(InputManager&);
	ImActionProxy(ImAction&, InputManager&);

	ImActionProxy(const ImActionProxy&) = delete;
	ImActionProxy(ImActionProxy&&);

	~ImActionProxy(); // deletes action from InputManager

	ImAction * getAction() const;
};

class InputManager {
	std::set<ImAction> actions;
	std::map<Keybind, std::reference_wrapper<ImAction>> bindings;

	// for actions that haven't been registered yet, and have custom keybinds
	std::map<std::pair<std::string, std::string>, Keybind> pendingRestores;

public:
	InputManager(std::string targetElem);

	ImActionProxy add(std::string ctx, std::string name, Keybind defaultKb, std::function<bool(const Keybind&, bool))>);
	void del(ImAction&);

	bool setKeybinding(std::string ctx, std::string name, Keybind);
};
