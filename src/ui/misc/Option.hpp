#pragma once

#include <type_traits>
#include "util/emsc/ui/Object.hpp"
#include "util/emsc/ui/EventHandle.hpp"
#include "util/NonCopyable.hpp"

template<typename T, typename = void>
class Option;

template<typename T>
class LabelledOption : public eui::Object {
	eui::Object label;
	Option<T> opt;

public:
	LabelledOption(T& _opt, std::string_view txt, bool labelBeforeOpt = false)
	: label("label"),
	  opt(_opt) {
		std::string_view sel = opt.getSelector();
		sel.remove_prefix(1); // remove '#'
		label.setAttribute("for", sel);
		setLabelText(txt);
		addClass("l-opt");

		if (labelBeforeOpt) {
			label.appendTo(*this);
			opt.appendTo(*this);
		} else {
			opt.appendTo(*this);
			label.appendTo(*this);
		}
	}

	void setLabelText(std::string_view txt) {
		label.setProperty("textContent", txt);
	}

	Option<T>& getOpt() {
		return opt;
	}

	eui::Object& getLabel() {
		return label;
	}
};

template<template<typename> class P>
class Option<P<bool>> : public eui::Object {
	using OptT = P<bool>;
	using SlotKey = typename OptT::SlotKey;

	OptT& opt;
	SlotKey sk;
	eui::EventHandle changeHdlr;

public:
	Option(OptT& _opt)
	: eui::Object("input"),
	  opt(_opt),
	  sk(opt.connect([this] (auto) { updateState(); })) {
		setAttribute("type", "checkbox");
		updateState();
		changeHdlr = createHandler("change", [this] {
			// should the slotKey be blocked instead?
			changeHdlr.setEnabled(false);
			opt = eui::Object::getPropertyBool("checked");
			changeHdlr.setEnabled(true);
			return false;
		}, false);
	}

	void updateState() {
		setPropertyBool("checked", opt);
	}
};

template<template<typename> class P, typename T>
class Option<P<T>, std::enable_if_t<std::is_arithmetic_v<T>>> : public eui::Object, NonCopyable {
	using OptT = P<T>;
	using SlotKey = typename OptT::SlotKey;

	OptT& opt;
	SlotKey sk;
	eui::EventHandle changeHdlr;

public:
	Option(OptT& _opt)
	: eui::Object("input"),
	  opt(_opt),
	  sk(opt.connect([this] (auto) { updateState(); })) {
		updateState();
		changeHdlr = createHandler("change", [this] {
			// should the slotKey be blocked instead?
			changeHdlr.setEnabled(false);
			auto p = getPropertyAs<T>("value");
			if (p.has_value()) {
				opt = p.value();
			} else {
				updateState();
			}

			changeHdlr.setEnabled(true);
			return false;
		}, false);
	}

	void updateState() {
		setProperty("value", toString<T>(opt));
	}

	void asRange(T min, T max, T step) {
		setAttribute("type", "range");
		setAttribute("min", toString<T>(min));
		setAttribute("max", toString<T>(max));
		setAttribute("step", toString<T>(step));
	}
};
