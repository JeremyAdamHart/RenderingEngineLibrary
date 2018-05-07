#pragma once

#include <GLGeometry.h>
#include <glm/glm.hpp>
#include <vector>
#include <tuple>
#include <glSupport.h>
#include "VertexBufferTemplates.h"
#include "TemplateParameterParsing.h"
#include "MultiBuffer.h"
#include "BufferQueue.h"

namespace renderlib {

using namespace glm;

/**
* STREAM GEOMETRY CLASS
**/
template<class... Ts>
class StreamGeometry : public GLGeometryContainer {
public:
	const int BUFFER_COPIES = 3;
	MultiBufferSwitch buffManager;
	vector<wrap_tuple<BufferQueue, Ts...>> queue;
protected:
	GLuint vao;
	size_t bufferSize;
	size_t elementNum;
	GLenum mode;
	std::vector<GLuint> vbo;
	std::vector<void*> voidVboPointers;
	GLsync drawSync;
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

	void createBufferStorage() {
		voidVboPointers.resize(vbo.size());
		allocateBufferStorage<Ts...>(vbo.data(), bufferSize*BUFFER_COPIES, voidVboPointers.data());
	}

public:
	StreamGeometry(GLenum mode = GL_TRIANGLES):bufferSize(0), elementNum(0), mode(mode){
		initVAO();
		createBufferStorage();
	}
	StreamGeometry(size_t bufferSize, size_t elementNum=0, GLenum mode = GL_TRIANGLES) :
		bufferSize(bufferSize), elementNum(elementNum), mode(mode) 
	{
		initVAO();
		createBufferStorage();
	}

	size_t getBufferSize() const { return bufferSize; }

	virtual GLuint getVboID(int num) const { return (num < vbo.size()) ? vbo[num] : 0; }

	virtual void drawGeometry() {
		static bool firstDraw = true;

		if (!firstDraw) {
			GLenum syncStatus = GL_UNSIGNALED;
			while (syncStatus != GL_ALREADY_SIGNALED && syncStatus != GL_CONDITION_SATISFIED) {
				syncStatus = glClientWaitSync(drawSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			//	printf("Waiting\n");
			}
		}

		int bufferNum = buffManager.getRead();
		printf("bufferNum %d\n", bufferNum);

		glBindVertexArray(vao);
		glDrawElementsBaseVertex(mode, elementNum, GL_UNSIGNED_INT, 0, bufferSize*bufferNum);
		//Syncronize
		glDeleteSync(drawSync);
		drawSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		buffManager.endRead();	//Free buffer to be written to
		firstDraw = false;
	}

	template<size_t N> 
	nth_type<N, Ts...>* vboPointer() { 
		size_t byteOffset = bufferSize * sizeof(nth_type<N, Ts...>)*buffManager.getWrite();
		return reinterpret_cast<nth_type<N, Ts...>*>((char*)voidVboPointers[N]); 
	}

	template<size_t N>
	void loadBuffer(nth_type<N, Ts...>* data){
		size_t byteSize = bufferSize * sizeof(nth_type<N, Ts...>);
		for (int i = 0; i < BUFFER_COPIES; i++) {
			std::memcpy((char*)voidVboPointers[N]+i*byteSize, data, byteSize);
		}
	}

/*	//DON'T USE
	template<unsigned int N, class D> 
	void loadBuffer(D* data, GLenum usage) {
//		BufferCompatability<N - 1, D, Ts...> compat;	
		VERIFY_TEMPLATE_TYPE_AT_INDEX(N, D, Ts...);		//Checks to see if Nth type in Ts is D

		glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(D)*bufferSize, data, usage);
	}

	//DON'T USE
	void loadGeometry(size_t newBufferSize, GLenum usage, Ts*... args) {
		bufferSize = newBufferSize;
		loadVBOs(0, &vbo, bufferSize, usage, args...);
	}
*/

	void loadElementArray(size_t newElementNum, GLenum usage, unsigned int* elements) {
		elementNum = newElementNum;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementNum * sizeof(unsigned int), elements, usage);
	}
};

}