#pragma once

#include <GLGeometry.h>
#include <GLObject.h>
#include "TemplateParameterParsing.h"
#include "VertexBufferTemplates.h"

#include "VertexAttribLayout.h"

namespace renderlib {

template<class T>
GLenum toGLenum() { return GL_NONE; }

template<> GLenum toGLenum<int>();
template<> GLenum toGLenum<unsigned int>();
template<> GLenum toGLenum<char>();
template<> GLenum toGLenum<unsigned char>();
template<> GLenum toGLenum<short>();
template<> GLenum toGLenum<unsigned short>();

template<typename T1>
constexpr bool usingInstancedImp() {
	return T1::Divisor > 0;
}

template<typename T1, typename T2, typename... Args>
constexpr bool usingInstancedImp() {
	if constexpr (T1::Divisor > 0)
		return true;
	else
		return usingInstancedImp<T2, Args...>();
}

template<typename... Args>
struct usingInstanced {
	static constexpr bool value = usingInstancedImp<Args...>();
};


template<class C, class T1>
void loadBuffers_rec(C* obj, typename const T1::Type* t1) {
	obj->loadBuffer<T1>(t1);
}

template<class C, class T1, class T2, class... Ts>
void loadBuffers_rec(C* obj, typename const T1::Type* t1, const typename T2::Type* t2, typename const Ts::Type*... args) {
	loadBuffers_rec<C, T1>(obj, t1);
	loadBuffers_rec<C, T2, Ts...>(obj, t2, args...);
}

///
/// Template class for geometry
/// @tparam Ts The list of vertex attributes provided
///
template<typename... Ts>
class GeometryT : public GLGeometryContainer {
protected:
	VertexBindingMapping vaoMap;
	unsigned int nextAttributeNumber;
	GLenum mode;
	std::vector<GLBuffer>vbo;
public:
	const static bool Elements = false;
	size_t bufferSize;
	size_t instanceCount;

	GeometryT(GLenum mode = GL_TRIANGLES) : bufferSize(0), mode(mode) {
		for (int i = 0; i < sizeof...(Ts); i++) {
			vbo.push_back(createBufferID());
			glBindBuffer(GL_ARRAY_BUFFER, vbo.back());
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	GeometryT(GLenum mode, typename const Ts::Type*... data, size_t dataSize, size_t dataInstances)
		: bufferSize(dataSize), instanceCount(dataInstances), mode(mode)
	{
		for (int i = 0; i < sizeof...(Ts); i++) {
			vbo.push_back(createBufferID());
			glBindBuffer(GL_ARRAY_BUFFER, vbo.back());
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		loadBuffers(data..., dataSize, dataInstances);
	}

	GeometryT(GLenum mode, typename const Ts::Type*... data, size_t dataSize)
		: GeometryT(mode, data..., dataSize, 1)
	{}

	///
	/// Loads buffer for a single attribute
	///@tparam A - The Attribute to load data into
	///@param data - Data to uploaded to the vertex buffer
	template<typename A>
	void loadBuffer(typename const A::Type* data) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[indexOf<A, Ts...>()]);
		
		unsigned int dataSize;
		if constexpr (A::Divisor > 0) dataSize = instanceCount;
			else 
		dataSize = bufferSize;

		glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(typename A::Type), data, GL_DYNAMIC_DRAW);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	template<typename A>
	void loadSubBuffer(typename const A::Type* data, size_t offset, size_t size) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[indexOf<A, Ts...>()]);

		unsigned int dataSize;
		if constexpr (A::Divisor > 0) dataSize = instanceCount;
		else
			dataSize = bufferSize;

		glBufferSubData(GL_ARRAY_BUFFER, offset *sizeof(typename A::Type), size * sizeof(typename A::Type), data);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	virtual void loadBuffers(typename const Ts::Type*... data, size_t dataSize) {
		loadBuffers(data..., dataSize, 1);
	}

	virtual void loadBuffers(typename const Ts::Type*... data, size_t dataSize, size_t dataInstances) {
		bufferSize = dataSize;
		instanceCount = dataInstances;
		loadBuffers_rec<GeometryT<Ts...>, Ts...>(this, data...);
	}

	virtual void drawGeometry(GLProgram program) override {
		
		vaoMap.requestVAO<Ts...>(program, &vbo);
		
		if constexpr (!usingInstanced<Ts...>::value)
			glDrawArrays(mode, 0, bufferSize);
		else
			glDrawArraysInstanced(mode, 0, bufferSize, instanceCount);
		glBindVertexArray(0);
	}
};

template<typename IndexT, typename... Ts>
class IndexGeometryT : public GeometryT<Ts...> {
protected:
	size_t indexSize;
public:
	IndexGeometryT(GLenum mode = GL_TRIANGLES) : GeometryT<Ts...>(mode) {
		vbo.push_back(createBufferID());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	IndexGeometryT(GLenum mode, const IndexT* indices, size_t indexSize, typename const Ts::Type*... data, size_t dataSize, size_t dataInstances)
		: GeometryT<Ts...>(mode, data..., dataSize, dataInstances)
	{
		vbo.push_back(createBufferID());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		loadIndices(indices, indexSize);
	}

	IndexGeometryT(GLenum mode, const IndexT* indices, size_t indexSize, typename const Ts::Type*... data, size_t dataSize)
		: GeometryT<Ts...>(mode, data..., dataSize, 1)
	{
		vbo.push_back(createBufferID());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		loadIndices(indices, indexSize);
	}

	void loadIndices(const IndexT* indices, size_t newIndexSize) {
		indexSize = newIndexSize;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize*sizeof(IndexT), indices, GL_DYNAMIC_DRAW);
	}

	virtual void drawGeometry(GLProgram program) override {
		vaoMap.requestVAO<Ts...>(program, &vbo);	//glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.back());		//Should be built into requestVAO
		if constexpr (!usingInstanced<Ts...>::value)
			glDrawElements(mode, indexSize, toGLenum<IndexT>(), 0);
		else
			glDrawElementsInstanced(mode, indexSize, toGLenum<IndexT>(), 0, instanceCount);
		glBindVertexArray(0);
	}
};

//Default IndexGeometry with unsigned int
template<typename... Args>
using IndexGeometryUint = IndexGeometryT<unsigned int, Args...>;
using TexNormalGeometry = GeometryT<attrib::Position, attrib::Normal, attrib::TexCoord>;
using TextureGeometry = GeometryT<attrib::Position, attrib::TexCoord>;
using NormalGeometry = GeometryT < attrib::Position, attrib::Normal>;
using PositionGeometry = GeometryT<attrib::Position>;
using PositionIndexGeometry = IndexGeometryUint<attrib::Position>;
using TexNormalIndexGeometry = IndexGeometryUint<attrib::Position, attrib::Normal, attrib::TexCoord>;
using TextureIndexGeometry = IndexGeometryUint<attrib::Position, attrib::TexCoord>;
using NormalIndexGeometry = IndexGeometryUint<attrib::Position, attrib::Normal>;

}
