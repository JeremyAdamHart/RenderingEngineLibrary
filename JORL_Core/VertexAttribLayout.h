#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#ifndef USING_GLEW
#include <glad/glad.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif
#include "GLObject.h"
#include "VertexBufferTemplates.h"

namespace renderlib {

template<typename T>
class Attribute {
public:
	using Type = T;
	static constexpr int Divisor = 0;
	static inline std::string name() { return ""; }
};

namespace attrib {
	struct Position :public Attribute<glm::vec3> {
		static inline std::string name() { return "VertexPosition"; }
	};

	struct Position2D :public Attribute<glm::vec2> {
		static inline std::string name() { return "VertexPosition2D"; }
	};

	struct Normal :public Attribute<glm::vec3> {
		static inline std::string name() { return "VertexNormal"; }
	};

	struct TexCoord :public Attribute<glm::vec2> {
		static inline std::string name() { return "VertexTexCoord"; }
	};

	struct Color :public Attribute<glm::vec4> {
		static inline std::string name() { return "VertexColor"; }
	};

	struct PointSize :public Attribute<float> {
		static inline std::string name() { return "PointSize"; }
	};

	struct Offset :public Attribute<glm::vec3> {
		static inline std::string name() { return "VertexOffset"; }
	};

	struct ColorIndex :public Attribute<unsigned char> {
		static inline std::string name() { return "VertexColorIndex"; }
	};

	template<typename T>
	struct Pinned : public T {
		using T::name;
		
		using Pinned_Type = void;
	};

	template <typename... Ts>
	using void_t = void;

	template <typename T, typename = void>
	struct usingPinned : std::false_type {};

	template <typename T>
	struct usingPinned <T, void_t<typename T::Pinned_Type>> : std::true_type {};

	template<typename T, int N>
	struct Instanced : public T {
		static constexpr int Divisor = N;
	};

	template<int N>
	struct InstanceModelMatrix : public Instanced<Attribute<glm::mat4>, N> {
		using Instanced<Attribute<glm::mat4>, N>::Divisor;
		using Instanced<Attribute<glm::mat4>, N>::Type;
		static inline std::string name() { return "InstanceModelMatrix"; }	//VertexModelMatrix?
	};

}

class VertexBinding {
public:
	VertexBinding();
	VertexBinding(const std::vector<int>& attribLocations);
	std::vector<int> attribLocations;

	bool operator==(const VertexBinding& other) const;
	bool operator!=(const VertexBinding& other) const;
};

template<typename T1>
void getAttribLocation(GLuint program, VertexBinding* bindings){
	bindings->attribLocations.push_back(glGetAttribLocation(program, T1::name().c_str()));
}

template<typename T1, typename T2, typename ...Ts>
void getAttribLocation(GLuint program, VertexBinding* bindings) {
	getAttribLocation<T1>(program, bindings);
	getAttribLocation<T2, Ts...>(program, bindings);
}

template<typename ...Ts>
VertexBinding createVertexBinding(GLuint program) {
	VertexBinding binding;
	getAttribLocation<Ts...>(program, &binding);
	return binding;
}

class VertexBindingMapping {
	std::unordered_map<GLuint, unsigned int> programMapping;
	std::vector<VertexBinding> attributeBindings;
	std::vector<GLVAO> vaos;
public:
	template<typename ...Ts>
	void requestVAO(GLProgram program, std::vector<GLBuffer>* vbos) {
		auto vao_it = programMapping.find(program);
		if (vao_it == programMapping.end()) {
			VertexBinding newBinding = createVertexBinding<Ts...>(program);
			bool found = false;
			for (unsigned int i = 0; i < attributeBindings.size(); i++) {
				if (newBinding == attributeBindings[i]) {
					programMapping[program] = i;
					found = true;
					break;
				}
			}
			if (!found) {
				GLVAO newVAO = createVAOID();
				glBindVertexArray(newVAO);
				initVertexAttributes<Ts...>(vbos->data(), newBinding.attribLocations.data());

				programMapping[program] = attributeBindings.size();
				attributeBindings.push_back(newBinding);
				vaos.push_back(newVAO);
			}
			else
				glBindVertexArray(vaos[programMapping[program]]);
		}
		else
			glBindVertexArray(vaos[programMapping[vao_it->first]]);


	}

};

};