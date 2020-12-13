#pragma once

#include <TemplatedShader.h>
#include <glm/glm.hpp>
#include <string>
#include <fmt/format.h>
#include <sstream>
#include <map>
#include <functional>
#include <exception>
#include <unordered_set>

std::vector<std::string> split(std::string str, char delim) {
	std::vector<std::string> pieces{ std::string("") };
	for (auto c : str) {
		if (c == delim) {
			if (pieces.back().size())
				pieces.push_back("");
		}
		else
			pieces.back().push_back(c);
	}

	return pieces;
}

class ShaderGenerator {

};

enum class ShaderLocation {
	Main,
	Function,
	Global
};

struct ShaderStage{
private:
	unsigned int id;
	ShaderStage(unsigned int id):id(id) { }
public:
	static ShaderStage Vertex() {    return ShaderStage(1u << 0); }
	static ShaderStage Tess_Eval() { return ShaderStage(1u << 1); }
	static ShaderStage Tess_Ctrl() { return ShaderStage(1u << 2); }
	static ShaderStage Geometry() {  return ShaderStage(1u << 3); }
	static ShaderStage Fragment() {  return ShaderStage(1u << 4); }
	static ShaderStage Any() {       
		return Vertex() + Fragment() + Tess_Eval() + Tess_Ctrl() + Geometry(); 
	}
//	static ShaderStage Input() {     return ShaderStage(1u << 4); }
//	static ShaderStage Body() {      return ShaderStage(1u << 5); }

	bool operator==(ShaderStage& other) const { return other.id == id; }
	bool operator<(ShaderStage& other) const { return id < other.id; }
	bool contains(ShaderStage& other) const { return id & other.id; }
	ShaderStage operator+(ShaderStage& other) const { return ShaderStage(id | other.id); }
	ShaderStage operator-(ShaderStage& other) const { return ShaderStage(id ^ other.id); }
};

#define fmtarg(a) fmt::arg(#a, a.name) 

class RuntimeTypeError : public std::exception {
	virtual const char* what() const throw()
	{
		return "Runtime type error";
	}
};

class Output;

class Input {
public:
	std::string name;
	const std::string type;
	Output* source;
	ShaderNode* parent;
	Input(const char* type, const char* name):name(name), type(type) {}
	operator bool() { return source != nullptr; }
	void operator=(Output& out) {
		if (type == out.type) {
			source = &out;
			out.sink = this;
		}
		else
			throw RuntimeTypeError();
	}
};

class Output {
public:
	const std::string type;
	Input* sink;
	ShaderNode* parent;
	std::string name;
	Output(const char* type, const char* name) :name(name), type(type) {}

	operator bool() { return sink != nullptr; }
};

struct ShaderText {
	std::string text;
	ShaderLocation location;
	ShaderText(std::string text, ShaderLocation location = ShaderLocation::Main) :text(text), location(location) {}
};

class ShaderNode {
	std::vector<std::tuple<ShaderStage, ShaderText>> contents;
public:
	std::map<std::string, Input> inputs;
	std::map<std::string, Output> outputs;

	ShaderNode(
		std::vector<std::string> inputVars,
		std::vector<std::string> outputVars,
		std::vector<std::tuple<ShaderStage, ShaderText>> contents //std::string contents,
		):contents(contents)	
	{
		for (auto input : inputVars) {
			auto type_name = split(input, ' ');	//[Type, Name]
			inputs.emplace(type_name[1], type_name[0].c_str(), type_name[1].c_str());
			inputs[type_name[1]].parent = this;
		}

		for (auto output : outputVars) {
			auto type_name = split(output, ' ');	//[Type, Name]
			outputs.emplace(type_name[1], type_name[0].c_str(), type_name[1].c_str());
			outputs[type_name[1]].parent = this;
		}
	}
	
	Input& in(std::string name) {
		auto it = inputs.find(name);
		assert(it == inputs.end() && fmt::format("{name} not a member on inputs", fmt::arg("name", name)));
		return it->second;
	}

	Output& out(std::string name) {
		auto it = outputs.find(name);
		assert(it == outputs.end() && fmt::format("{name} not a member on outputs", fmt::arg("name", name)));
		return it->second;
	}

	//Currently slow
	//Uses format in_{name} or out_{name}
	std::string autoFormat(std::string str) const {
		for (auto const&[name, input] : inputs)
			str = fmt::format(str, fmt::arg(("in_"+name).c_str(), input.name.c_str()));
		for (auto const&[name, output] : outputs)
			str = fmt::format(str, fmt::arg(("out_"+name).c_str(), output.name.c_str()));
	}
	//Change interface, shouldn't require argument.                                     
	virtual std::string getText(ShaderStage shaderStage) {
		for (auto[stage, text] : contents) {
			if(stage.contains(shaderStage))
				return autoFormat()
		}
	}

	virtual void preDraw(renderlib::GLProgram program) {}

	virtual std::string scope(std::string text) {
		return std::string("{\n") + text + std::string("}\n");
	}
};

std::vector<ShaderNode*> getTotalOrdering(ShaderNode& endNode) {
	return {};
}

class ShaderContents {
	std::unordered_set<std::string> mainVariables;
	int genericVariableCounter = 0;
	std::unordered_set<std::string> outputVariables;

	//Faster than with other variables
	std::string addGenericVariable(std::unordered_set<std::string>& variables) {
		std::string varName;
		do {
			varName = std::string("var_") + std::to_string(genericVariableCounter++);
		} while (variables.count(varName));

		return varName;
	}

	std::string addVariable(std::unordered_set<std::string>& variables, std::string name = "") {
		if (name == "") {
			return addGenericVariable(variables);
		}

		std::string varName = name;
		int count = 0;
		while (variables.count(varName))
			varName = name + std::to_string(count++);

		return varName;
	}
public:
	std::map<ShaderLocation, std::string> location;

	std::string addMainVariable(std::string name = "") {
		return addVariable(mainVariables, name);
	}

	std::string addOutputVariable(std::string name = "") {
		return addVariable(outputVariables, name);	//Add output. to this or do elsewhere?
	}
};

renderlib::GLProgram createProgram(ShaderNode& endNode) {
	auto nodeOrdering = getTotalOrdering(endNode);

	//@TODO Determine if there's a tessellation or geometry shader
	std::map<ShaderStage, ShaderContents> shaderStages;

	for (auto node : nodeOrdering) {
		node.
	}
}

namespace node {

ShaderNode createMat3Transform() {
	return ShaderNode(
		{
			"mat3 m",
			"vec3 p"
		},
		{
			"vec3 p"
		},
		{ {ShaderStage::Any(), "{out_p} = {in_m}*{in_p};"} }
	);
}

ShaderNode createMat4Transform() {

	return ShaderNode(
		{
			"mat4 m",
			"vec4 point4f",
			"vec3 point3f",
			"vec3 vector"
		},
		{
			"vec3 point3f",
			"vec4 point4f"
		},
		{ {ShaderStage::Any(),
		R"(
#ifdef INPUT_point3f
	vec4 p = vec4({in_point3f}, 1.0);
#elif defined(INPUT_point4f)
	vec4 p = {in_point4f};
#elif defined(INPUT_point4f)
	vec4 p = vec4({in_point3f}, 0.0);
#endif
	p = {in_m}*p;
#ifdef OUTPUT_point3f
	{out_point3f} = p.xyz/p.w;
#elif defined(OUTPUT_point4f)
	{out_point4f} = p;
)"} }
);
}

ShaderNode createSampler2D() {
	return ShaderNode(
		{
			"vec2 uv",
			"texture2D texture"
		},
		{
			"float grayscale"
			"vec3 color",
			"float alpha"
		},
		{ {ShaderStage::Any(),
		R"(
vec4 color = texture({in_texture}, {in_uv}\n";
#ifdef OUTPUT_grayscale
	{out_grayscale} = color.r;
#endif
#ifdef OUTPUT_color
	{out_color} = color.rgb;
#endif
#ifdef OUTPUT_alpha
	{out_alpha} = color.a;
#endif
		)"} }
	);

}

ShaderNode createBlinnPhong(ShaderStage stage=ShaderStage::Fragment()) {
	return ShaderNode(
		{
			"vec3 color",
			"vec3 view_position",
			"vec3 light_position",
			"vec3 position"
			"vec3 normal"
			"float ks",
			"float kd",
			"float ambient",
			"float specular_exponent"
		},
		{
			"vec3 color"
		},
		{ {stage,
		R"(
vec3 toViewer = normalize({in_view_position} - {in_position});
vec3 toLight = normalize({in_light_position} - {in_position});

vec3 h = normalize(toViewer + toLight);
float lambertion = clamp(dot({in_normal}, toLight), 0.0, 1.0);

float alpha = {in_specular_exponent};
{out_color} = ({in_ks}*(alpha+2.0)*(alpha+4.0)
	/(8.0*M_PI*(exp2(-alpha/2.0)+alpha)) 
	* clamp(pow(dot({in_normal}, h), alpha), 0.0, 1.0)*lambertion
	+ {in_kd}*lambertion + {in_ambient})*{in_color};
)"} }
	);
}



}