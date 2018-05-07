#pragma once

#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include "Scene.h"
#include <vector>
#include <glm/glm.hpp>

//Defines accepted: USING_TEXTURE

namespace renderlib {

class AOShader : public Shader {
protected:
	vector<int> uniformLocations;

	void calculateUniformLocations();
	void loadUniforms(glm::vec3 camera_pos, glm::vec3 cam_right, 
		glm::vec3 cam_up, glm::vec3 light_pos);
public:
	AOShader(map<GLenum, string> defines = map<GLenum, string>{});

	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});

	void draw(const Camera &cam, glm::vec3 lightPos, Drawable &obj);		//Change to use pointer to drawable?
	void draw(const Camera &cam, glm::vec3 lightPos, Scene &scene);
};

}