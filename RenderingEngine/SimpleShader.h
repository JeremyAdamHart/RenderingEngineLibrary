#pragma once

#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include "Scene.h"
#include <vector>

class SimpleShader : public Shader {
protected:
	vector<int> uniformLocations;

	void calculateUniformLocations();
	void loadUniforms(const glm::mat4& vp_matrix, const glm::mat4& m_matrix);
public:
	SimpleShader(map<GLenum, string> defines = {});
	
	virtual bool createProgram(map<GLenum, string> defines = {});

	void draw(const Camera &cam, const Drawable &obj);
	void draw(const Camera &cam, const Scene &scene);
};