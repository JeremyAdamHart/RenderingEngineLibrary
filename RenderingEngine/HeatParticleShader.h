#pragma once
#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include "Scene.h"
#include <vector>

class HeatParticleShader : public renderlib::Shader {
protected:
	std::vector<int> uniformLocations;

	void calculateUniformLocations();
	void loadUniforms(const glm::mat4& vp_matrix, const glm::mat4& inv_rot_matrix);
public:
	HeatParticleShader(std::map<GLenum, std::string> defines = std::map<GLenum, std::string>{});

	virtual bool createProgram(std::map<GLenum, std::string> defines = std::map<GLenum, std::string>{});

	void draw(const renderlib::Camera &cam, renderlib::Drawable &obj);
};
