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
	MultiBufferSwitch buffManager;
protected:
	GLuint vao;
	size_t bufferSize;
	size_t elementNum;
	GLenum mode;
	std::vector<GLuint> vbo;
	std::vector<void*> voidVboPointers; 
	GLsync drawSync;
	GLuint vboElement;
	std::vector<wrap_tuple<BufferQueue, Ts...>> queue;
	std::vector<char> streamed;

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
		allocateBufferStorage<Ts...>(vbo.data(), streamed.data(), bufferSize*MultiBufferSwitch::BUFFER_COPIES, voidVboPointers.data());
	}

public:
	StreamGeometry(GLenum mode = GL_TRIANGLES):
		bufferSize(0), elementNum(0), mode(mode), queue(MultiBufferSwitch::BUFFER_COPIES), 
		streamed({}) 
	{
		initVAO();
		streamed.resize(vbo.size(), true);
		createBufferStorage();
	}
	StreamGeometry(size_t bufferSize, std::vector<char> streamed = {}, size_t elementNum = 0, GLenum mode = GL_TRIANGLES) :
		bufferSize(bufferSize), elementNum(elementNum), mode(mode),
		queue(MultiBufferSwitch::BUFFER_COPIES), streamed(streamed)
	{
		initVAO();
		streamed.resize(vbo.size(), true);
		createBufferStorage();
	}

	template<size_t N>
	void modify(size_t index, const nth_type<N, Ts...>& val) {
		if (index > bufferSize)
			printf("Bad\n");
		for (int i = 0; i < queue.size(); i++) {
			get<N, BufferQueue, Ts...>(queue[i])->add(index, val);
		}
	}

	template<size_t N>
	void dump() {
		size_t bufferNum = buffManager.getWrite();
		nth_type<N, Ts...>* data = vboPointer<N>();
		get<N, BufferQueue, Ts...>(queue[bufferNum])->dump(data);
	}

	size_t getBufferSize() const { return bufferSize; }

	template<size_t N>
	void unmap() {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	virtual GLuint getVboID(int num) const { return (num < vbo.size()) ? vbo[num] : 0; }

	virtual void drawGeometry() {
		static bool firstDraw = true;
		
/*		if (!firstDraw) {
			GLenum syncStatus = GL_UNSIGNALED;
			int numMilliseconds = 0;
			while (syncStatus != GL_ALREADY_SIGNALED && syncStatus != GL_CONDITION_SATISFIED) {
				syncStatus = glClientWaitSync(drawSync, GL_SYNC_FLUSH_COMMANDS_BIT, 1000);
				numMilliseconds++;
			}
		}*/
		buffManager.endRead();	//Free buffer to be written to
		int bufferNum = buffManager.getRead();

		glBindVertexArray(vao);
		glDrawElementsBaseVertex(mode, elementNum, GL_UNSIGNED_INT, 0, bufferSize*bufferNum);
		//Syncronize
//		glDeleteSync(drawSync);
	//	drawSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

		firstDraw = false;
	}

	template<size_t N> 
	nth_type<N, Ts...>* vboPointer() { 
		size_t offset = bufferSize*buffManager.getWrite();
		return reinterpret_cast<nth_type<N, Ts...>*>((char*)voidVboPointers[N]) + offset; 
	}

	template<size_t N>
	void loadBuffer(nth_type<N, Ts...>* data) {
		if (streamed[N]) {
			size_t byteSize = bufferSize * sizeof(nth_type<N, Ts...>);
			for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
				std::memcpy((char*)voidVboPointers[N]+i*byteSize, data, byteSize);
			}
		}
		else {
			glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
			std::vector<nth_type<N, Ts...>> buffer;

			for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
				for (int j = 0; j < bufferSize; j++) {
					buffer.push_back(data[j]);
				}
			}
			glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(nth_type<N, Ts...>), buffer.data(), GL_DYNAMIC_DRAW);
		}

		checkGLErrors("StreamGeometry::loadBuffer()");
	}
/*
	template<>
	void loadBuffer<0>(glm::vec3* data) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		std::vector<glm::vec3> buffer;

		for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
			for (int j = 0; j < bufferSize; j++) {
				buffer.push_back(data[j]);
			}
		}
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(glm::vec3), buffer.data(), GL_DYNAMIC_DRAW);
	}

	template<>
	void loadBuffer<1>(glm::vec3* data) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		std::vector<glm::vec3> buffer;

		for (int i = 0; i < MultiBufferSwitch::BUFFER_COPIES; i++) {
			for (int j = 0; j < bufferSize; j++) {
				buffer.push_back(data[j]);
			}
		}
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(glm::vec3), buffer.data(), GL_DYNAMIC_DRAW);
	}
	*/
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