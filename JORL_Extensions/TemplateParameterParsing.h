#pragma once

#define VERIFY_TEMPLATE_TYPE_AT_INDEX(index, type, list) verifyNthTemplateArg<index, type, list> checkTemplateTypeAtIndex

template<unsigned int N, class Arg, class Head, class... List> class verifyNthTemplateArg {
	verifyNthTemplateArg <N - 1, Arg, List...>* recursiveCheck;
};

template<class Arg, class... List> class verifyNthTemplateArg <0, Arg, Arg, List...> {};


//Shamelessly taken from: https://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
template<std::size_t I, typename T, typename...Ts> 
struct nth_element_impl {
	using type = typename nth_element_impl<I-1, Ts...>::type;
};

template<typename T, typename...Ts>
struct nth_element_impl<0, T, Ts...> {
	using type = T;
};

template <std::size_t N, typename ...Ts>
using nth_type = typename nth_element_impl<N, Ts...>::type;

//^^^ Below is me again
template<size_t I, typename X, typename T>
constexpr size_t indexOfImp() {
	if constexpr (std::is_same<T, X>::value) {
		return I;
	}
	else {
		static_assert(std::is_same<T, X>::value, "Type not present in list of indexOf()");
	}
}

template<size_t I, typename X, typename T1, typename T2, typename ...Ts>
constexpr size_t indexOfImp() {
	if constexpr (std::is_same<T1, X>::value)
		return I;
	else
		return indexOfImp<I + 1, X, T2, Ts...>();
}

template<typename X, typename ...Ts>
constexpr size_t indexOf() { return  indexOfImp<0, X, Ts...>(); }



//PARAMETER PACK WRAPPER
//Wraps parameter pack inside other class
//template<class C, class T, class ...Ts>
//using type_wrap = <Ts...>;


//TEMPLATE CLASS LIST
//Makes list of classes with the given templates
// ie: a list of vectors
//template<template<class C> class W, class ...Ts> class wrap_tuple;


template<template<typename> typename W, typename T1, typename ...Ts>
struct wrap_tuple {
	W<T1> item;
	wrap_tuple<W, Ts...> tail;
};

template<template<typename> typename W, typename T>
struct wrap_tuple<W, T>{
	W<T> item;
};

template<size_t N, template<typename> typename W, typename T, typename ...Ts>
struct get_imp{
	static auto value(wrap_tuple<W, T, Ts...>& t) {
		return get_imp<N-1, W, Ts...>::value(t.tail);
	}
};

template<template<typename> typename W, typename T, typename ...Ts>
struct get_imp <0, W, T, Ts...>{
	static auto value(wrap_tuple<W, T, Ts...>& t) {
		return &(t.item);
	}
};

//https://gist.github.com/IvanVergiliev/9639530	-- Reference
template<size_t N, template<typename> typename W, typename T, typename ...Ts>
auto get(wrap_tuple<W, T, Ts...>& t) {
	return get_imp<N, W, T, Ts...>::value(t);
}