#pragma once

#include <jorl/core/Shader.h>
#include <jorl/core/Drawable.h>
#include <jorl/core/Camera.h>
#include <jorl/core/Scene.h>

#include <vector>

namespace renderlib {

class SimpleTexShader : public Shader {
protected:
	vector<int> uniformLocations;

	void calculateUniformLocations();
	void loadUniforms(const glm::mat4& vp_matrix, const glm::mat4& m_matrix);
public:
	SimpleTexShader(map<GLenum, string> defines = map<GLenum, string>{});

	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});

	void draw(const Camera &cam, Drawable &obj);
	void draw(const Camera &cam, const Scene &scene);
};

}