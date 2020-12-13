#pragma once

#include <TemplatedShader.h>
#include <glm/glm.hpp>
#include <string>
#include <fmt/format.h>
#include <sstream>

template<typename T> class Output;

class ShaderGenerator {

};

class ShaderStage {
public:
	std::stringstream main;
	std::stringstream functions;
	std::stringstream inputs;
	std::stringstream outputs;
};

#define fmtarg(a) fmt::arg(#a, a.name) 

class ShaderNode {
	
	virtual void activatePath()
};

struct InputBase {
	std::string name;
	
};

template<typename T>
struct Input {
	std::string name;
	Output<T>* source;
	ShaderNode* parent;
};

template<typename T>
struct Output {
	std::string name;
	Input<T>* sink;
	ShaderNode* parent;

	operator bool() { return sink != nullptr; }
};

class MatrixTransform3 {
	struct In {
		Input<glm::mat4> m;
		Input<glm::vec3> p;
	} in;

	struct Out {
		Output<glm::vec3> p;
	} out;

	void addToShader(ShaderStage shader) {
		if (out.p) {
			shader.main << fmt::format("{out.p} = {in.m}*{in.p}", fmtarg(out.p), fmtarg(in.p), fmtarg(in.m));
		}
	}
};

class VertexTransform {
public:
	struct In {
		Input<glm::mat4> P;
		Input<glm::mat4> V;
		Input<glm::mat4> M;
		Input<glm::mat4> MV;
		Input<glm::mat4> VP;
		Input<glm::mat4> MVP;
		Input<glm::vec4> position;
		Input<glm::vec3> normal;
		Input<glm::vec2> uv;
	} in;

	struct Out {
		Output<glm::vec4> position;
		Output<glm::vec3> normal;
		Output<glm::vec2> uv;
	} out;

	void addToShader() {
		if (out.position) {
			
		}
	}
};

class BlinnPhong {
public:
	struct In {
		Input <glm::vec4> position;
		Input<glm::vec3> normal;
		Input<glm::vec2> uv;

	} in;
	struct Out {
		Output<glm::vec4> color;
	} out;
};



/*
class Node {
	
};*/

class ShaderNode {
	
};





template<typename T>
class Node {
	T data;
};

template<typename A, typename B>
class Link {
public:
	std::shared_ptr<Node<A>> input;
	std::shared_ptr<Node<B>> output;
};

TessGeometry tg;
BPShader bp;
connect(tg.position, bp.position);
connect(tg.normal, bp.normal);
bp.position = tg.position;
bp.normal = tg.normal;
tg.position = 
auto& [tg.position, tg.
