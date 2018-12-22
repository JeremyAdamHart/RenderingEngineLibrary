#pragma once

//////////////////
// NON-COMPLIANT TO ATTRIB_LOCATION
//////////////////

#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include "Scene.h"
#include <vector>

namespace renderlib {

class VRColorShader : public Shader {
protected:
	vector<int> uniformLocations;

	void calculateUniformLocations();
	void loadUniforms(const glm::mat4& vp_matrix, const glm::mat4& m_matrix, glm::vec3 viewPosition, glm::vec3 lightPos, 
		float fogScale, float fogDistance, glm::vec3 fogColor);
public:
	VRColorShader(int maxColorNum);
	VRColorShader(map<GLenum, string> defines = map<GLenum, string>{});

	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});
	virtual bool createNewProgram(vector<pair<GLenum, string>> shaderNames, map<GLenum, string> defines = map<GLenum, string>{});


	void draw(const Camera &cam, glm::vec3 lightPos, 
		float fogScale, float fogDistance, glm::vec3 fogColor, Drawable &obj);
	void draw(const Camera &cam, const Scene &scene);
};

}