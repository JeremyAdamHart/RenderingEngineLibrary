#pragma once

#include <GLGeometry.h>
#include <GLObject.h>
#include "TemplateParameterParsing.h"
#include "VertexBufferTemplates.h"

namespace renderlib {

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

	void resize(size_t newSize) { bufferSize = newSize; }

	template<size_t N>
	void loadBuffer(nth_type<N, Ts...>* data) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[N]);
		glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(nth_type<N, Ts...>), data, GL_DYNAMIC_DRAW);
	}

	virtual void drawGeometry() {
		glBindVertexArray(vao);
		glDrawArrays(mode, 0, bufferSize);
	}

};

template<class... Ts>
class ElementGeometryT : public GLGeometryContainer {
protected:
	GLuint vao;
	size_t bufferSize;
};





}