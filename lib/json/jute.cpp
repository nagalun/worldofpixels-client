#include "jute.h"

using namespace std;
using namespace jute;

string deserialize(const string& ref) {
	string out = "";
	for (size_t i = 0; i < ref.length(); i++) {
		if (ref[i] == '\\' && i + 1 < ref.length()) {
			int plus = 2;
			if (ref[i + 1] == '\"') {
				out += '"';
			} else if (ref[i + 1] == '\\') {
				out += '\\';
			} else if (ref[i + 1] == '/') {
				out += '/';
			} else if (ref[i + 1] == 'b') {
				out += '\b';
			} else if (ref[i + 1] == 'f') {
				out += '\f';
			} else if (ref[i + 1] == 'n') {
				out += '\n';
			} else if (ref[i + 1] == 'r') {
				out += '\r';
			} else if (ref[i + 1] == 't') {
				out += '\t';
			} else if (ref[i + 1] == 'u' && i + 5 < ref.length()) {
				unsigned long long v = 0;
				for (int j = 0; j < 4; j++) {
					v *= 16;
					if (ref[i + 2 + j] <= '9' && ref[i + 2 + j] >= '0') v += ref[i + 2 + j] - '0';
					if (ref[i + 2 + j] <= 'f' && ref[i + 2 + j] >= 'a') v += ref[i + 2 + j] - 'a' + 10;
				}
				out += (char)v;
				plus = 6;
			}
			i += plus - 1;
			continue;
		}
		out += ref[i];
	}
	return out;
}

string jValue::makesp(int d) const {
	string s = "";
	while (d--)
		s += "  ";
	return s;
}

string jValue::to_string_d(int d) const {
	if (type == JSTRING) return string("\"") + svalue + string("\"");
	if (type == JNUMBER) return svalue;
	if (type == JBOOLEAN) return svalue;
	if (type == JNULL) return "null";
	if (type == JOBJECT) {
		string s = string("{\n");
		for (size_t i = 0; i < properties.size(); i++) {
			s += makesp(d) + string("\"") + properties[i].first + string("\": ") +
			     properties[i].second.to_string_d(d + 1) + string(i == properties.size() - 1 ? "" : ",") + string("\n");
		}
		s += makesp(d - 1) + string("}");
		return s;
	}
	if (type == JARRAY) {
		string s = "[";
		for (size_t i = 0; i < arr.size(); i++) {
			if (i) s += ", ";
			s += arr[i].to_string_d(d + 1);
		}
		s += "]";
		return s;
	}
	return "##";
}

jValue::jValue() {
	this->type = JUNKNOWN;
}

jValue::jValue(jType tp) {
	this->type = tp;
}

string jValue::to_string() const {
	return to_string_d(1);
}

jType jValue::get_type() const {
	return type;
}

bool jValue::is_array() const {
	return get_type() == JARRAY;
}

bool jValue::is_string() const {
	return get_type() == JSTRING;
}

bool jValue::is_object() const {
	return get_type() == JOBJECT;
}

bool jValue::is_number() const {
	return get_type() == JNUMBER;
}

bool jValue::is_boolean() const {
	return get_type() == JBOOLEAN;
}

void jValue::set_type(jType tp) {
	type = tp;
}

void jValue::add_property(string_view key, jValue v) {
	mpindex.insert_or_assign(std::string(key), properties.size());
	properties.push_back(make_pair(std::string(key), v));
}

void jValue::add_element(jValue v) {
	arr.push_back(v);
}

void jValue::set_string(string_view s) {
	svalue = s;
}

double jValue::as_double(double def) const {
	char* end = nullptr;
	if (type != JNUMBER || svalue.empty()) return def;
	double n = std::strtod(svalue.c_str(), &end);
	return end != svalue.c_str() + svalue.size() ? def : n;
}

bool jValue::as_bool(bool def) const {
	if (type == JUNKNOWN) return def;
	return svalue == "true";
}

string jValue::as_string(std::string def) const {
	return type == JSTRING ? deserialize(svalue) : def;
}

std::vector<jValue>::iterator jValue::begin() {
	return arr.begin();
}

std::vector<jValue>::iterator jValue::end() {
	return arr.end();
}

std::vector<jValue>::const_iterator jValue::cbegin() const {
	return arr.cbegin();
}

std::vector<jValue>::const_iterator jValue::cend() const {
	return arr.cend();
}

int jValue::size() const {
	if (type == JARRAY) {
		return (int)arr.size();
	}
	if (type == JOBJECT) {
		return (int)properties.size();
	}
	return 0;
}

jValue jValue::operator[](int i) const {
	if (type == JARRAY) {
		return arr[i];
	}
	if (type == JOBJECT) {
		return properties[i].second;
	}
	return jValue();
}

jValue jValue::operator[](string_view s) const {
	auto it = mpindex.find(s);
	if (it == mpindex.end()) return jValue();
	return properties[it->second].second;
}

struct parser::token {
	string_view value;
	token_type type;

	token(string_view value = "", token_type type = UNKNOWN)
	: value(value),
	  type(type) { }
};

bool parser::is_whitespace(const char c) {
	return isspace(c);
}

int parser::next_whitespace(string_view source, int i) {
	while (i < (int)source.length()) {
		if (source[i] == '"') {
			i++;
			while (i < (int)source.length() && (source[i] != '"' || source[i - 1] == '\\'))
				i++;
		}
		if (is_whitespace(source[i])) return i;
		i++;
	}
	return (int)source.length();
}

int parser::skip_whitespaces(string_view source, int i) {
	while (i < (int)source.length()) {
		if (!is_whitespace(source[i])) return i;
		i++;
	}
	return -1;
}

vector<parser::token> parser::tokenize(string_view source) {
	// source += " ";
	vector<token> tokens;
	int index = skip_whitespaces(source, 0);
	while (index >= 0) {
		int next = next_whitespace(source, index);
		string_view str = source.substr(index, next - index);

		size_t k = 0;
		while (k < str.length()) {
			if (str[k] == '"') {
				size_t tmp_k = k + 1;
				while (tmp_k < str.length() && (str[tmp_k] != '"' || str[tmp_k - 1] == '\\'))
					tmp_k++;
				tokens.push_back(token(str.substr(k + 1, tmp_k - k - 1), STRING));
				k = tmp_k + 1;
				continue;
			}
			if (str[k] == '\'') {
				size_t tmp_k = k + 1;
				while (tmp_k < str.length() && (str[tmp_k] != '\'' || str[tmp_k - 1] == '\\'))
					tmp_k++;
				tokens.push_back(token(str.substr(k + 1, tmp_k - k - 1), STRING));
				k = tmp_k + 1;
				continue;
			}
			if (str[k] == ',') {
				tokens.push_back(token(",", COMMA));
				k++;
				continue;
			}
			if (str[k] == 't' && k + 3 < str.length() && str.substr(k, 4) == "true") {
				tokens.push_back(token("true", BOOLEAN));
				k += 4;
				continue;
			}
			if (str[k] == 'f' && k + 4 < str.length() && str.substr(k, 5) == "false") {
				tokens.push_back(token("false", BOOLEAN));
				k += 5;
				continue;
			}
			if (str[k] == 'n' && k + 3 < str.length() && str.substr(k, 4) == "null") {
				tokens.push_back(token("null", NUL));
				k += 4;
				continue;
			}
			if (str[k] == '}') {
				tokens.push_back(token("}", CROUSH_CLOSE));
				k++;
				continue;
			}
			if (str[k] == '{') {
				tokens.push_back(token("{", CROUSH_OPEN));
				k++;
				continue;
			}
			if (str[k] == ']') {
				tokens.push_back(token("]", BRACKET_CLOSE));
				k++;
				continue;
			}
			if (str[k] == '[') {
				tokens.push_back(token("[", BRACKET_OPEN));
				k++;
				continue;
			}
			if (str[k] == ':') {
				tokens.push_back(token(":", COLON));
				k++;
				continue;
			}
			if (str[k] == '-' || (str[k] <= '9' && str[k] >= '0')) {
				size_t tmp_k = k;
				if (str[tmp_k] == '-') tmp_k++;
				while (tmp_k < str.size() && ((str[tmp_k] <= '9' && str[tmp_k] >= '0') || str[tmp_k] == '.'))
					tmp_k++;
				tokens.push_back(token(str.substr(k, tmp_k - k), NUMBER));
				k = tmp_k;
				continue;
			}
			tokens.push_back(token(str.substr(k), UNKNOWN));
			k = str.length();
		}

		index = skip_whitespaces(source, next);
	}
	// for (int i=0;i<tokens.size();i++) {
	// cout << i << " " << tokens[i].value << endl;
	// }
	return tokens;
}

jValue parser::json_parse(vector<token> v, int i, int& r) {
	jValue current;
	if (v[i].type == CROUSH_OPEN) {
		current.set_type(JOBJECT);
		int k = i + 1;
		while (v[k].type != CROUSH_CLOSE) {
			string_view key = v[k].value;
			k += 2; // k+1 should be ':'
			int j = k;
			jValue vv = json_parse(v, k, j);
			current.add_property(key, vv);
			k = j;
			if (v[k].type == COMMA) k++;
		}
		r = k + 1;
		return current;
	}
	if (v[i].type == BRACKET_OPEN) {
		current.set_type(JARRAY);
		int k = i + 1;
		while (v[k].type != BRACKET_CLOSE) {
			int j = k;
			jValue vv = json_parse(v, k, j);
			current.add_element(vv);
			k = j;
			if (v[k].type == COMMA) k++;
		}
		r = k + 1;
		return current;
	}
	if (v[i].type == NUMBER) {
		current.set_type(JNUMBER);
		current.set_string(v[i].value);
		r = i + 1;
		return current;
	}
	if (v[i].type == STRING) {
		current.set_type(JSTRING);
		current.set_string(v[i].value);
		r = i + 1;
		return current;
	}
	if (v[i].type == BOOLEAN) {
		current.set_type(JBOOLEAN);
		current.set_string(v[i].value);
		r = i + 1;
		return current;
	}
	if (v[i].type == NUL) {
		current.set_type(JNULL);
		current.set_string("null");
		r = i + 1;
		return current;
	}
	return current;
}

jValue parser::parse(string_view str) {
	int k;
	return json_parse(tokenize(str), 0, k);
}
