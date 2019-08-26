#pragma once

#include <GLGeometry.h>
#include <GLObject.h>
#include "TemplateParameterParsing.h"
#include "VertexBufferTemplates.h"

namespace renderlib {

template<size_t N, class C, class T1>
void loadBuffers_rec(C* obj, T1* t1) {
	obj->loadBuffer<N>(t1);
}

template<size_t N, class C, class T1, class T2, class... Args>
void loadBuffers_rec(C* obj, T1* t1, T2* t2, Args*... args) {
	loadBuffers_rec<N>(obj, t1);
	loadBuffers_rec<N+1>(obj, t2, args...);
}

template<class... Ts>
class GeometryT : public GLGeometryContainer {
protected:
	GLVAO vao;
	size_t bufferSize;
	GLenum mode;
	std::vector<GLBuffer>vbo;

	bool initVAO() {
		glBindVertexArray(vao);
		bool result = initVertexBuffers<Ts...>(&vbo);
		glBindVertexArray(0);

		return result;
	}
public:
	GeometryT(GLenum mode = GL_TRIANGLES) : vao(createVAOID()), bufferSize(0), mode(mode){
		initVAO();
	}

	GeometryT(GLenum mode, Ts*... data, size_t dataSize)
		: vao(createVAOID()), bufferSize(dataSize), mode(mode)
	{
		initVAO();
		loadBuffers(data..., dataSize);
	}

	void resize(size_t newSize) { bufferSize = newSize; }

	template<size_t N>
	void loadBuffer(nth_type<N, Ts...>* data) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
		glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(nth_type<N, Ts...>), data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void loadBuffers(Ts*... data, size_t dataSize) {
		resize(dataSize);
		loadBuffers_rec<0>(this, data...);
	}

	virtual void drawGeometry() {
		glBindVertexArray(vao);
		glDrawArrays(mode, 0, bufferSize);
		glBindVertexArray(0);
	}

};

template<class T>
GLenum toGLenum() { return GL_NONE; }

template<> GLenum toGLenum<int>();
template<> GLenum toGLenum<unsigned int>();
template<> GLenum toGLenum<char>();
template<> GLenum toGLenum<unsigned char>();
template<> GLenum toGLenum<short>();
template<> GLenum toGLenum<unsigned short>();

template<class I, class... Ts>
class ElementGeometryT : public GLGeometryContainer {
protected:
	GLVAO vao;
	size_t bufferSize;
	size_t indexSize;
	GLenum mode;
	std::vector<GLBuffer>vbo;

	bool initVAO() {
		glBindVertexArray(vao);
		bool result = initVertexBuffers<Ts...>(&vbo);
		vbo.push_back(createBufferID());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());
		glBindVertexArray(0);

		return result;
	}
public:
	ElementGeometryT(GLenum mode = GL_TRIANGLES) : vao(createVAOID()), bufferSize(0), indexSize(0), mode(mode) {
		initVAO();
	}
	
	ElementGeometryT(GLenum mode, I* indices, size_t indexSize, Ts*... data, size_t dataSize) 
		: vao(createVAOID()), bufferSize(dataSize), indexSize(indexSize), mode(mode) 
	{
		initVAO();
		loadBuffers(data..., dataSize);
		loadIndices(indices, indexSize);
	}
	//template<class ...Args>
	//ElementGeometryT()

	void resizeData(size_t newSize) { bufferSize = newSize; }
	void resizeIndices(size_t newSize) { indexSize = newSize; }

	void loadBuffers(Ts*... data, size_t dataSize) {
		resizeData(dataSize);
		loadBuffers_rec<0>(this, data...);
	}

	template<size_t N>
	void loadBuffer(nth_type<N, Ts...>* data) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
		glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(nth_type<N, Ts...>), data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	void loadIndices(I* indices, size_t size) {
		indexSize = size;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(I), indices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	virtual void drawGeometry() {
		glBindVertexArray(vao);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());
		glDrawElements(mode, indexSize, toGLenum<I>(), 0);
		glBindVertexArray(0);
	}

};





}