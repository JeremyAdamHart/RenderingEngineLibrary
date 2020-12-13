#pragma once

#include <ShaderGeneratorImp2.h>

template<typename T>
class Dirty {
	T value;
	bool dirty;
public:
	Dirty(T value) :value(value), dirty(true) {}

	const T& get() {
		dirty = false;
		return value; 
	}
	void set(T value) {
		dirty = true;
		this->value = value;
	}
	operator bool() const { return dirty; }
};

template<typename T>
std::string type_name();

template<>
std::string type_name<float>() { return "float"; }
template<>
std::string type_name<glm::vec3>() { return "vec3"; }
template<>
std::string type_name<glm::vec2>() { return "vec2"; }
template<>
std::string type_name<glm::vec4>() { return "vec4"; }
template<>
std::string type_name<int>() { return "int"; }
template<>
std::string type_name<glm::mat4>() { return "mat4"; }
template<>
std::string type_name<glm::mat3>() { return "mat3"; }

template<typename T>
void load_uniform(GLint loc, T value);

template<>
void load_uniform<float>(GLint loc, float value) { glUniform1f(loc, value); }
template<>
void load_uniform<glm::vec2>(GLint loc, glm::vec2 value) { glUniform2f(loc, value.x, value.y); }
template<>
void load_uniform<glm::vec3>(GLint loc, glm::vec3 value) { glUniform3f(loc, value.x, value.y, value.z); }
template<>
void load_uniform<glm::vec4>(GLint loc, glm::vec4 value) { glUniform4f(loc, value.x, value.y, value.z, value.w); }
template<>
void load_uniform<int>(GLint loc, int value) { glUniform1i(loc, value); }
template<>
void load_uniform<unsigned int>(GLint loc, unsigned int value) { glUniform1ui(loc, value); }
template<>
void load_uniform<glm::mat3>(GLint loc, glm::mat3 value) { glUniformMatrix3fv(loc, 1, false, &value[0][0]); }
template<>
void load_uniform<glm::mat4>(GLint loc, glm::mat4 value) { glUniformMatrix4fv(loc, 1, false, &value[0][0]); }


template<typename T>
class UniformNode : public ShaderNode{
public:
	Dirty<T> value;
	GLint location;
	UniformNode(std::string type_and_name) : ShaderNode({}, { type_and_name }, "", "")
	{}

	virtual void preDraw(renderlib::GLProgram program) override {
		load_uniform<T>(value.get());
	}
};

template<typename T>
ShaderNode createUniform(const char* name) {
	return UniformNode(fmt::format("{type} {name}", 
		fmt::arg("type", type_name<T>()), 
		fmt::arg("name", name)));
}

	
