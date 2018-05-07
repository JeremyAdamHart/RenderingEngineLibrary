#pragma once

#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include <vector>
namespace renderlib {

class PerlinNoiseShader2D : public Shader {
protected:
	vector<int> uniformLocations;

	void calculateUniformLocations();
	void loadUniforms(const glm::mat4& vp_matrix, const glm::mat4& m_matrix);
public:
	PerlinNoiseShader2D(map<GLenum, string> defines = map<GLenum, string>{});

	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});

	void draw(const Camera &cam, Drawable &obj);
};

}
