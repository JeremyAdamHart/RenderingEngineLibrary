#pragma once

#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include "Scene.h"
#include <vector>
#include <glm/glm.hpp>

namespace renderlib {

enum class PNShaderOutput{
	POSITION=0,
	POSITION_AND_NORMAL
};

class PosNormalShader : public Shader {
protected:
	vector<int> uniformLocations;

	void calculateUniformLocations();
	void loadUniforms(const glm::mat4& vp_matrix, const glm::mat4& m_matrix);
public:
	enum {
		POSITION=0,
		POSITION_AND_NORMAL
	};

	PosNormalShader(map<GLenum, string> defines = map<GLenum, string>{});
	PosNormalShader(PNShaderOutput output);

	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});

	void draw(const Camera &cam, glm::vec3 lightPos, Drawable &obj);		//Change to use pointer to drawable?
	void draw(const Camera &cam, glm::vec3 lightPos, Scene &scene);
};

}