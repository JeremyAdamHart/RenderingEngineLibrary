#pragma once

#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include "Scene.h"
#include <vector>
#include <glm/glm.hpp>

//Defines accepted: USING_TEXTURE

class TorranceSparrowShader : public Shader {
protected:
	vector<int> uniformLocations;

	bool usingTexture;

	void calculateUniformLocations();
	void loadUniforms(const glm::mat4& vp_matrix, 
		const glm::mat4& m_matrix, glm::vec3 camera_pos, glm::vec3 light_pos);
public:
	TorranceSparrowShader(map<GLenum, string> defines = {});

	virtual bool createProgram(map<GLenum, string> defines = {});

	void draw(const Camera &cam, glm::vec3 lightPos, const Drawable &obj);
	void draw(const Camera &cam, glm::vec3 lightPos, Scene &scene);
};