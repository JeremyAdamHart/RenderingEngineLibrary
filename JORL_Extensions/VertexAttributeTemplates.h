#pragma once

#include <string>
#include <utility>

namespace renderlib {

namespace StringTemplate {

template <char... chars>
using tstring = std::integer_sequence<char, chars...>;

template <typename T, T... chars>
constexpr tstring<chars...> operator""_tstr() { return { }; }

template <typename>
struct Name;

template <char... elements>
struct Name<tstring<elements...>> {
	static const char* GetString() {
		constexpr static const char str[sizeof...(elements) + 1] = { elements..., '\0' };
		return str;
	}
};


}

namespace Attrib {


inline const char* Position() { return "Position"; }
inline const char* Normal() { return "Normal"; }

template<const char* NameFunc(void), typename Type>
class ID{
	static std::string name() { return std::string(NameFunc()); }
}








}
}