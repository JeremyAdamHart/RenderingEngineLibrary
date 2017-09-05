#pragma once

#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include "Scene.h"
#include <vector>
#include <glm/glm.hpp>

namespace renderlib {

class PosNormalShader : public Shader {
protected:
	vector<int> uniformLocations;

	void calculateUniformLocations();
	void loadUniforms(const glm::mat4& vp_matrix, const glm::mat4& m_matrix);
public:
	PosNormalShader(map<GLenum, string> defines = {});

	virtual bool createProgram(map<GLenum, string> defines = {});

	void draw(const Camera &cam, glm::vec3 lightPos, const Drawable &obj);		//Change to use pointer to drawable?
	void draw(const Camera &cam, glm::vec3 lightPos, Scene &scene);
};

}