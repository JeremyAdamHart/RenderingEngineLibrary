#pragma once

#include "Shader.h"
#include "Drawable.h"
#include "Camera.h"
#include "Scene.h"
#include "TemplatedShader.h"
#include "ShadedMat.h"
#include "ColorMat.h"
#include <vector>
#include <glm/glm.hpp>


//Defines accepted: USING_TEXTURE

namespace renderlib {

enum class BPTextureUsage {
	TEXTURE,
	NONE
};

class BlinnPhongShaderT : public ShaderT<ShadedMat, ColorMat> {
public:
	BlinnPhongShaderT();
	
	void draw(const Camera &cam, glm::vec3 lightPos, Drawable &obj);
	void drawVertexBinding(const Camera& cam, glm::vec3 lightPos, Drawable &obj);
};

class BlinnPhongShader : public Shader {
protected:

	bool usingTexture;

	void calculateUniformLocations();
	void loadUniforms(const glm::mat4& vp_matrix, 
		const glm::mat4& m_matrix, glm::vec3 camera_pos, glm::vec3 light_pos);
public:
	BlinnPhongShader(BPTextureUsage texUsage);
	BlinnPhongShader(map<GLenum, std::string> defines = map<GLenum, std::string>{});

	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});
//	virtual bool createNewProgram(vector<pair<GLenum, string>> shaderNames, map<GLenum, string> defines = map<GLenum, string>{});

	void draw(const Camera &cam, glm::vec3 lightPos, Drawable &obj);		//Change to use pointer to drawable?
	void draw(const Camera &cam, glm::vec3 lightPos, Scene &scene);
};

}