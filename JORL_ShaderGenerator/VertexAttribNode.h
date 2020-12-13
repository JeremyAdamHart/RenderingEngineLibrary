#pragma once

#include <ShaderGeneratorImp2.h>
#include <VertexAttribLayout.h>
#include <UniformShaderNode.h>
#include <fmt/format.h>

template<typename T>
class AttributeNode : public ShaderNode {
public:
	AttributeNode(std::string type_and_name) : ShaderNode({}, { type_and_name }, { {ShaderStage::Vertex(), ShaderText()} })
	{}
};

namespace node {

template<typename Attrib>
ShaderNode createAttribute() {
	return AttributeNode(std::string(type_name<Attrib::T>()) + " " + std::string(Attrib::name()));
}

//template<typename ...Ts>
//void attributeLayoutNodeText_imp(std::vector<std::tuple<ShaderStage, ShaderText>>* text);

template<typename T>
void attributeLayoutOutputs_imp(std::vector<std::string>* text) {
	text->push_back(
		std::string(type_name<T::Type>()) + 
		" " + 
		std::string(T::name()));
}

template<typename T1, typename T2, typename ...Ts>
void attributeLayoutOutputs_imp(std::vector<std::string>* text) {
	attributeLayoutNodeText_imp<T1>(text);
	attributeLayoutNodeText_imp<T2, Ts...>(text);
}
//*/
/*
template<typename T>
void attributeLayoutNodeText_imp(std::vector<std::tuple<ShaderStage, ShaderText>>* text) {
	text->push_back({ ShaderStage::Vertex(), std::string(type_name<T::Type>()) + " " + std::string(T::name()))
}
//*/

//std::vector<std::tuple<ShaderStage, ShaderText>>

///*
template<typename ...Ts>
std::vector<std::string> attributeLayoutOutputs() {
	std::vector<std::string> outputText;
	attributeLayoutOutputs_imp<Ts...>(&outputText);

	return layoutNodeText;
}
//*/

template<typename T>
size_t attributeSize();

template<typename T1, typename T2, typename ...Ts>
std::string attributeLayoutString_imp(unsigned int *nextIndex) {
	std::string text = attributeLayoutString_imp<T1>(nextIndex);
	return text + attributeLayoutString_imp<T2, Ts...>(nextIndex);
}

template<typename T>
std::string attributeLayoutString_imp(unsigned int *nextIndex) {
	int index = *nextIndex;
	*nextIndex += attributeSize<T>();
	return fmt::format("layout(location = {index}) in {type} {name}\n",
		fmt::arg("index", index), 
		fmt::arg("type", type_name<T::Type>()), 
		fmt::arg("name", T::name()));
}

template<typename ...Ts>
std::string attributeLayoutString() {
	int index = 0;
	return attributeLayoutString_imp<Ts...>(index)
}

template<typename ...Ts>
class AttributesNode : public ShaderNode {
public:
	AttributesNode() : ShaderNode({}, attributeLayoutOutputs<Ts...>(),
		{ {ShaderStage::Vertex(), ShaderText(attributeLayoutString<Ts...>(), ShaderLocation::Global)} })
	{}
};

}