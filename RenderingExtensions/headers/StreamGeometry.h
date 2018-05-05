#pragma once

#include <GLGeometry.h>
#include <glm/glm.hpp>
#include <vector>
#include <glSupport.h>
#include "VertexBufferTemplates.h"
#include "TemplateParameterParsing.h"
#include "MultiBuffer.h"

namespace renderlib {

using namespace glm;

/**
* STREAM GEOMETRY CLASS
**/
template<class... Ts>
class StreamGeometry : public GLGeometryContainer {
protected:
	GLuint vao;
	size_t bufferSize;
	size_t elementNum;
	GLenum mode;
	std::vector<GLuint> vbo;
	std::vector<void*> voidVboPointers;
	std::vector<std::vector<GLsync>> syncObjects;
	MultiBufferSwitch buffManager;
	GLuint vboElement;

	std::vector<unsigned int> vboCopies;
	
	bool initVAO() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		initVertexBuffers<Ts...>(&vbo);
		glGenBuffers(1, &vboElement);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);

		glBindVertexArray(0);

		return checkGLErrors("StreamGeometry::initVAO()");
	}

public:
	const int BUFFER_COPIES = 3;

	StreamGeometry(GLenum mode = GL_TRIANGLES):bufferSize(0), elementNum(0), mode(mode){
		initVAO();
//		createBufferStorage();
	}
	StreamGeometry(size_t bufferSize, size_t elementNum, GLenum mode = GL_TRIANGLES) :
		bufferSize(bufferSize), elementNum(elementNum), mode(mode) 
	{
		initVAO();
//		createBufferStorage();
	}

	virtual GLuint getVboID(int num) const { return (num < vbo.size()) ? vbo[num] : 0; }

	virtual void drawGeometry() const {
		glBindVertexArray(vao);
		glDrawElements(mode, elementNum, GL_UNSIGNED_INT, 0);
	}

	template<unsigned int N, class D> 
	nth_type<N, Ts...>* vboPointer() { return nth_type<N, Ts...>*(voidVboPointers[N]); }

	void createBufferStorage() {
		voidVboPointers.resize(vbo.size());
		allocateBufferStorage<Ts...>(vbo.data(), bufferSize*BUFFER_COPIES, vboVoidPointers.data());
	}

	//Change to using "nth_element"
	template<unsigned int N, class D> 
	void loadBuffer(D* data, GLenum usage) {
//		BufferCompatability<N - 1, D, Ts...> compat;	
		VERIFY_TEMPLATE_TYPE_AT_INDEX(N, D, Ts...);		//Checks to see if Nth type in Ts is D

		glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(D)*bufferSize, data, usage);
	}

	//Call this version
	void loadGeometry(size_t newBufferSize, GLenum usage, Ts*... args) {
		bufferSize = newBufferSize;
		loadVBOs(0, &vbo, bufferSize, usage, args...);
	}




	void loadElementArray(size_t newElementNum, GLenum usage, unsigned int* elements) {
		elementNum = newElementNum;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementNum * sizeof(unsigned int), elements, usage);
	}
};

}