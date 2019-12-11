#pragma once

#include <TemplatedShader.h>
#include <ShadedMat.h>
#include <ColorMat.h>
#include <TextureMat.h>
#include <Camera.h>

namespace renderlib {

struct DepthMapTag {};

using DepthTextureMat = TextureMatT<DepthMapTag>;

class DepthVarianceShader : public ShaderT<> {
public:
	DepthVarianceShader();

	void draw(Camera& cam, Drawable& obj);
};

template<typename ColorSrc>
class ShadowShader : public ShaderT<ShadedMat, ColorSrc>{
private:
	enum {
		VP_MATRIX_LOCATION = ShadedMat::COUNT
		+ 1,
		M_MATRIX_LOCATION,
		CAMERA_POS_LOCATION,
		LIGHT_POS_LOCATION,
		LIGHT_MATRIX_LOCATION,
		DEPTH_TEXTURE_LOCATION,
		LIGHT_MV_MATRIX_LOCATION,
		COUNT
	};

	static std::vector<std::pair<GLenum, std::string>> defaultShaders () {
		return  {{GL_VERTEX_SHADER, "shaders/bpShaded.vert"}, {GL_FRAGMENT_SHADER, "shaders/shadowShader.frag"}};
	}
public:
/*	ShadowShader():
		ShaderT<ShadedMat, ColorSrc>(
			{ {GL_VERTEX_SHADER, "shaders/bpShader.vert"}, {GL_FRAGMENT_SHADER, "shaders/shadowShader.frag"} },
			{ (std::is_same<ColorSrc, TextureMat>::value) ? {GL_FRAGMENT_SHADER, "#define USING_TEXTURE\n"} : {} },
			{ "ka", "kd", "ks", "alpha", (std::is_same<ColorSrc, TextureMat>::value) ? "colorTexture" : "color",
  			"view_projection_matrix", "model_matrix", "camera_position", "lightPos",
			"light_matrix", "depthTexture"})
	{}
*/	
	template<typename ColorSrc>
	constexpr const char* uniformName() {
		return "";
	}

	template<>
	constexpr const char* uniformName<ColorMat>() { return "color"; }

	template<>
	constexpr const char* uniformName<TextureMat>() { return "colorTexture"; }

	ShadowShader(std::vector<std::pair<GLenum, std::string>> shaders=defaultShaders(), 
		std::map<GLenum, std::string> defines = {}) 
		: ShaderT<ShadedMat, ColorSrc>(
			shaders,
			defines,
			{ "ka", "kd", "ks", "alpha", uniformName<ColorSrc>(),
			"view_projection_matrix", "model_matrix", "camera_position", "lightPos",
			"light_matrix", "depthTexture", "light_mv_matrix" })
	{}

	void draw(const Camera &cam, glm::vec3 lightPos, Camera &lightCam, Texture depthTexture, Drawable &obj) {
		glUseProgram(programID);

		mat4 vp_matrix = cam.getProjectionMatrix()*cam.getCameraMatrix();
		mat4 m_matrix = obj.getTransform();
		mat4 l_matrix = lightCam.getProjectionMatrix()*lightCam.getCameraMatrix();
		mat4 l_mv_matrix = lightCam.getCameraMatrix();
		vec3 camera_pos = cam.getPosition();

		loadMaterialUniforms(obj);
		glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
		glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
		glUniform3f(uniformLocations[CAMERA_POS_LOCATION], camera_pos.x, camera_pos.y, camera_pos.z);
		glUniform3f(uniformLocations[LIGHT_POS_LOCATION],
			lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(uniformLocations[LIGHT_MATRIX_LOCATION], 1, false, &l_matrix[0][0]);
		glUniform1i(uniformLocations[DEPTH_TEXTURE_LOCATION], depthTexture.getTexUnit());
		glUniformMatrix4fv(uniformLocations[LIGHT_MV_MATRIX_LOCATION], 1, false, &l_mv_matrix[0][0]);

		obj.getGeometry().drawGeometry(programID);
		glUseProgram(0);
	}
};

}