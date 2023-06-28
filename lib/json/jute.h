#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <charconv>

namespace jute {
enum jType { JSTRING, JOBJECT, JARRAY, JBOOLEAN, JNUMBER, JNULL, JUNKNOWN };

class jValue {
private:
	std::string svalue;
	jType type;
	std::vector<std::pair<std::string, jValue>> properties;
	std::map<std::string, size_t, std::less<>> mpindex;
	std::vector<jValue> arr;


public:
	jValue();
	jValue(jType);
	std::string to_string() const;
	jType get_type() const;
	bool is_array() const;
	bool is_string() const;
	bool is_object() const;
	bool is_number() const;
	bool is_boolean() const;
	void set_type(jType);
	void add_property(std::string_view key, jValue v);
	void add_element(jValue v);
	void set_string(std::string_view s);

	template<typename T>
	T as_int(T def = {0}) const {
		const std::string& s = svalue;
		if (type != JNUMBER || s.empty()) {
			return def;
		}

		T n;
		auto res = std::from_chars(s.data(), s.data() + s.size(), n);

		if (res.ptr != s.data() + s.size() || res.ec != std::errc()) { // contains extra data, or too big
			return def;
		}

		return n;
	}

	double as_double(double def = 0.0) const;
	bool as_bool(bool def = false) const;
	std::string as_string(std::string def = {}) const;
	std::vector<jValue>::iterator begin();
	std::vector<jValue>::iterator end();
	std::vector<jValue>::const_iterator cbegin() const;
	std::vector<jValue>::const_iterator cend() const;
	int size() const;
	jValue operator[](int i) const;
	jValue operator[](std::string_view s) const;

private:
	std::string makesp(int) const;
	std::string to_string_d(int) const;
};

class parser {
private:
	enum token_type {
		UNKNOWN,
		STRING,
		NUMBER,
		CROUSH_OPEN,
		CROUSH_CLOSE,
		BRACKET_OPEN,
		BRACKET_CLOSE,
		COMMA,
		COLON,
		BOOLEAN,
		NUL
	};

	struct token;
	static bool is_whitespace(const char c);
	static int next_whitespace(std::string_view source, int i);
	static int skip_whitespaces(std::string_view source, int i);

	static std::vector<token> tokenize(std::string_view source);
	static jValue json_parse(std::vector<token> v, int i, int& r);

public:
	static jValue parse(std::string_view str);
};
} // namespace jute
