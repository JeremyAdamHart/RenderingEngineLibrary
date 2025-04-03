#include "GlowShader.h"

namespace renderlib {

std::vector<pair<GLenum, string>> flatShaderNames = {
	{GL_VERTEX_SHADER, "shaders/flatColorShader.vert"}, {GL_FRAGMENT_SHADER, "shaders/flatColorShader.frag" } };

FlatColorShader::FlatColorShader() :ShaderT<ColorMat>(flatShaderNames, {}, { "color", "ModelViewProjectionMatrix" }) {

}

void FlatColorShader::draw(Camera& cam, Drawable obj) {
	if (!programID) {
		printf("FlatColorShader: Shader compilation failure\n");
		return;
	}

	glUseProgram(programID);

	const size_t MODEL_VIEW_PROJECTION_LOC = ColorMat::COUNT;

	loadMaterialUniforms(obj);
	glm::mat4 modelViewProjectionMatrix = cam.getProjectionMatrix()*cam.getCameraMatrix()*obj.getTransform();
	glUniformMatrix4fv(uniformLocations[MODEL_VIEW_PROJECTION_LOC], 1, false, &modelViewProjectionMatrix[0][0]);

	obj.getGeometry().drawGeometry(programID);

	glUseProgram(0);
}

std::vector<pair<GLenum, string>> gaussianShaderNames = {
	{ GL_VERTEX_SHADER, "shaders/gaussian.vert" },{ GL_FRAGMENT_SHADER, "shaders/gaussian.frag" } };

enum {
	SIGMA=TextureMat::COUNT,
	N,
	DIRECTION,
	COUNT
};

GaussianBlurShader::GaussianBlurShader() : ShaderT<TextureMat>(gaussianShaderNames, {}, {"colorTex", "sigma", "n", "blurDirection" }) 
{
	
}

void GaussianBlurShader::draw(float sigma, int n, Direction direction, Drawable& obj) {
	if (!programID) {
		printf("GaussianBLurShader: Shader program invalid\n");
		return;
	}

	glUseProgram(programID);

	loadMaterialUniforms(obj);

	glUniform1f(uniformLocations[SIGMA], sigma);
	glUniform1i(uniformLocations[N], n);
	glUniform1i(uniformLocations[DIRECTION], int(direction));

	obj.getGeometry().drawGeometry(programID);

	glUseProgram(0);
}

}