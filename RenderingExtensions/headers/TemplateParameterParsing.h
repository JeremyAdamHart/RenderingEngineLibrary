#pragma once

#define VERIFY_TEMPLATE_TYPE_AT_INDEX(index, type, list) verifyNthTemplateArg<index, type, list> checkTemplateTypeAtIndex

template<unsigned int N, class Arg, class Head, class... List> class verifyNthTemplateArg {
	verifyNthTemplateArg <N - 1, Arg, List...>* recursiveCheck;
};

template<class Arg, class... List> class verifyNthTemplateArg <0, Arg, Arg, List...> {};


//Shamelessly taken from: https://ldionne.com/2015/11/29/efficient-parameter-pack-indexing/
template<size_t I, class T, class ...Ts> 
struct nth_element_impl {
	using type = class nth_element_impl<I - 1, Ts...>::type;
};

template<class T, class ...Ts>
struct nth_element_impl<0, T, Ts...> {
	using type = T;
};

template <size_t N, typename ...Ts>
using nth_type = typename nth_element_impl<N, Ts...>::type;