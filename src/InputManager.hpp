#pragma once

#include <util/explints.hpp>
#include <utility>
#include <functional>
#include <map>
#include <set>
#include <list>
#include <string>
#include <memory>
#include <vector>
#include <array>


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

// guarantee balance of PRESS/RELEASE calls, if listening on both (not for touch pointers due to multiple fingers...)

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
struct EmscriptenKeyboardEvent;
struct EmscriptenFocusEvent;
struct EmscriptenMouseEvent;
struct EmscriptenTouchEvent;
struct EmscriptenWheelEvent;

enum EPointerButtons {
	P_NONE       = 0,

	P_MPRIMARY   = 1,
	P_MSECONDARY = 2,
	P_MMIDDLE    = 4,
	P_MFOURTH    = 8,
	P_MFIFTH     = 16,

	/*P_MWHEELUP    = 32,
	P_MWHEELDOWN  = 64,
	P_MWHEELLEFT  = 128,
	P_MWHEELRIGHT = 256*/
};

enum EKeyModifiers {
	M_NONE  = 0,
	M_CTRL  = 1,
	M_ALT   = 2,
	M_SHIFT = 4,
	M_META  = 8,

	M_ALL   = 255
};

enum EActionTriggers : u8 {
	T_ONPRESS   = 1,
	T_ONHOLD    = 2,
	T_ONRELEASE = 4,
	T_ONCANCEL  = 8,  // action should be cancelled/reset

	T_ONWHEEL   = 16,
	T_ONMOVE    = 32, // pointer move
	T_ONENTER   = 64,  // new pointer entered screen
	T_ONLEAVE   = 128  // pointer left screen
};

class Keybind {
protected:
	//13/4/20!!! let keybind have a single button, can be mouse (not specific), keyboard btn (including modifier buttons)
	// TODO: deduplicate across multiple keybinds?
	std::string button;
	EKeyModifiers mods;

public:
	Keybind(EKeyModifiers, std::string);
	Keybind(EKeyModifiers, const char *);
	Keybind(EKeyModifiers, EPointerButtons button);
	Keybind(std::string);
	Keybind(const char *);
	Keybind(EPointerButtons button);

	const std::string& getButton() const;
	EKeyModifiers getModifiers() const;

	bool looseMatch(EKeyModifiers, EPointerButtons btn) const;
	bool looseMatch(EKeyModifiers, const char * key) const;
	bool looseMatch(EKeyModifiers, const std::string& key) const;

	// always strict checking
	bool operator ==(const Keybind&) const;
	bool operator  <(const Keybind&) const;
};

class ImAction {
public:
	class Event {
		EActionTriggers activationType;
		bool rejected;

	public:
		Event(EActionTriggers);
		Event(const Event&) = delete;
		Event(Event&&) = delete;

		EActionTriggers getActivationType() const;
		void reject();

		friend ImAction;
	};

private:
	const std::string name;
	std::vector<Keybind> bindings;
	std::function<void(Event& e, const InputInfo& ii)> cb;
	InputAdapter& adapter;
	bool enabled;
	bool bindingsChanged;
	EActionTriggers trg;

public:
	ImAction(InputAdapter&, std::string name, u8 triggers,
		std::function<void(Event& e, const InputInfo&)> = nullptr);

	ImAction(InputAdapter&, std::string name, // T_ONPRESS by default
		std::function<void(Event&, const InputInfo&)> = nullptr);

	~ImAction();

	const std::string& getName() const;
	EActionTriggers getTriggers() const;
	bool isEnabled() const;
	bool haveBindingsChanged() const;

	template<typename T>
	const Keybind * getMatch(EKeyModifiers, const T key) const;
	const Keybind * getMatch(EKeyModifiers, const InputInfo& key) const;

	std::vector<Keybind>& getBindings();
	const std::vector<Keybind>& getBindings() const;

	void addKeybind(Keybind);
	void setDefaultKeybind(Keybind);
	void setDefaultKeybinds(std::vector<Keybind>);
	void setEnabled(bool);
	void setCb(std::function<void(Event& e, const InputInfo& ii)>);
	void clearBindingsChanged();

	bool operator ()(EActionTriggers activationType, const InputInfo& ii);
	bool operator ==(const ImAction&) const;
};

class InputInfo { // passed to action callbacks
public:
	class Pointer {
	public:
		enum EType {
			MOUSE,
			TOUCH
		};

	private:
		int lastX;
		int lastY;
		int x;
		int y;
		EPointerButtons btns;
		EType type;
		bool present;

	public:
		Pointer();

		int getX() const;
		int getY() const;
		int getDx() const;
		int getDy() const;
		int getLastX() const;
		int getLastY() const;
		EPointerButtons getButtons() const;
		EType getType() const;
		bool isPresent() const;
		bool isActive() const;
		void finishMoving();
		void set(int x, int y, EPointerButtons, EType);
		void set(int x, int y, EType);
		void set(EPointerButtons, EType);
		void setPresent(bool);
	};

private:
	double wheelDx;
	double wheelDy;
	mutable std::array<Pointer, 8> pointers;
	mutable std::vector<Pointer *> pointersPresent;
	Pointer * updatedPointer;
	EKeyModifiers currentModifiers;
	mutable bool ptrListOutdated;

public:
	InputInfo();

	EKeyModifiers getModifiers() const;

	double getWheelDx() const;
	double getWheelDy() const;
	int getX() const;
	int getY() const;
	int getDx() const;
	int getDy() const;
	int getLastX() const;
	int getLastY() const;
	// midpoint funcs, avg between all pointers
	float getMidX() const;
	float getMidY() const;
	float getMidDx() const;
	float getMidDy() const;
	float getLastMidX() const;
	float getLastMidY() const;
	EPointerButtons getButtons() const;
	Pointer::EType getType() const;

	int getNumPointers() const; // pointers inside viewport (present)
	int getNumActivePointers() const; // pointers with any pressed buttons (active)
	const std::vector<Pointer *>& getPointers() const; // active pointers ordered before present
	// not specifying id returns last updated pointer
	const Pointer& getPointer(int id = -1) const;

protected:
	Pointer& getPointer(int id = -1);
	void finishMoving();
	void setModifiers(EKeyModifiers);
	void setWheel(double, double);
};

class InputStorage {
	// for actions that haven't been registered yet, and have custom keybinds
	// can include restores for sub-actions, example:
	// Base/Tool/Cursor/Draw => P_MPRIMARY
	// Last section of the string is always the action name
	std::multimap<std::string, Keybind> savedBindings;

public:
	InputStorage(); // load from localStorage
	~InputStorage(); // save to localStorage

	void popStoredKeybinds(const std::string&, std::function<void(Keybind)>);
	void storeKeybinds(const std::string&, std::vector<Keybind>&);
};

class InputAdapter {
	const InputManager& im;
	InputAdapter * const parentAdapter;
	InputStorage& storage;
	const std::string context;
	const int priority;

	std::vector<ImAction *> actions;
	// Holds actions which were sent a T_ONPRESS event (and may receive others, if specified)
	std::vector<ImAction *> activeActions;

	std::list<InputAdapter> linkedAdapters;

	bool enabled;

public:
	InputAdapter(InputAdapter * parent, InputStorage& is, std::string context, int priority = 0);
	InputAdapter(const InputAdapter&) = delete;
	~InputAdapter();

	const InputManager& getInputManager() const;
	const std::string& getContext() const;
	std::string getFullContext() const; // very slow
	InputAdapter& mkAdapter(std::string context, int priority = 0);

	// calls HOLD for T_ONHOLD-registered and active (held) keybinds
	void tick(const InputInfo&) const;

	template<typename T>
	bool matchDown(const T key, const InputInfo&);

	void matchEvent(EActionTriggers, const InputInfo&) const;

	template<typename T>
	bool matchUp(const T key, EActionTriggers lastTrigger, const InputInfo&);

	void releaseAll(const InputInfo& ii);

	void add(ImAction *);
	void del(ImAction *);

	bool operator <(const InputAdapter&) const;
};

// Order of destruction of derived classes will guarantee InputStorage is still
// valid when InputAdapter gets destroyed, allowing saving
class InputManager : InputInfo, InputStorage, public InputAdapter {
	using Ptr = InputInfo::Pointer;

	const char * kbTargetElement;
	const char * ptrTargetElement;

	// only holds either T_ONPRESS or T_ONRELEASE
	EActionTriggers lastTrigger;

public:
	InputManager(const char * kbTargetElement, const char * ptrTargetElement);
	~InputManager();

	const InputInfo& getLastInputInfo() const;

	void tick();

	bool keyDown(const char * key);
	bool keyUp(const char * key);

	bool pointerDown(int id, Ptr::EType, EPointerButtons changed, EPointerButtons buttons, bool fireEvent = true);
	bool pointerUp(int id, Ptr::EType, EPointerButtons changed, EPointerButtons buttons, bool fireEvent = true);
	void pointerMove(int id, Ptr::EType, int x, int y, bool fireEvent = true);
	void pointerCancel(int id, Ptr::EType, bool fireEvent = true);
	void pointerEnter(int id, Ptr::EType, bool fireEvent = true);
	void pointerLeave(int id, Ptr::EType, bool fireEvent = true);

	bool wheel(double dx, double dy, int type);

	void setModifiers(bool ctrl, bool alt, bool shift, bool meta);

	void lostFocus();

private:
	static int handleKeyEvent(int type, const EmscriptenKeyboardEvent * ev, void * data);
	static int handleMouseEvent(int type, const EmscriptenMouseEvent * ev, void * data);
	static int handleTouchEvent(int type, const EmscriptenTouchEvent * ev, void * data);
	static int handleFocusEvent(int type, const EmscriptenFocusEvent * ev, void * data);
	static int handleWheelEvent(int type, const EmscriptenWheelEvent * ev, void * data);
};
