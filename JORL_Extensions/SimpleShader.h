#pragma once

#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include "Scene.h"
#include <vector>

namespace renderlib {

class SimpleShader : public Shader {
protected:
	virtual vector<pair<GLenum, string>> defaultShaders();

	void loadUniforms(const glm::mat4& vp_matrix, const glm::mat4& m_matrix);
public:
	SimpleShader(map<GLenum, string> defines = map<GLenum, string>{});
	SimpleShader(vector<pair<GLenum, string>> alt_shaders, 
		map<GLenum, string> defines = map<GLenum, string>{});
	
	virtual std::vector<std::string> getUniformNames() override;

	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});

	virtual void draw(const Camera &cam, Drawable &obj);
	void draw(const Camera &cam, const Scene &scene);
};

}