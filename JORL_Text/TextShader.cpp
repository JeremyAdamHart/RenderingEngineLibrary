#include "TextShader.h"

namespace renderlib {

enum {
	MODEL_VIEW_PROJECTION=TextureMat::COUNT + ColorMat::COUNT,
	INVERSE_ROTATION_MATRIX
};

std::vector<pair<GLenum, string>> textShaderNames = {
	{GL_VERTEX_SHADER, "shaders/basicText.vert"}, {GL_FRAGMENT_SHADER, "shaders/basicText.frag" } };

TextShader::TextShader() :ShaderT<TextureMat, ColorMat>(textShaderNames, {}, { "colorTexture", "textColor", "model_view_projection_matrix", "inverse_rotation_matrix" })
{}

void TextShader::draw(Camera& cam, Drawable& obj) {
	if (!programID) {
		printf("TextShader: Shader compilation failure\n");
		return;
	}

	glUseProgram(programID);

	loadMaterialUniforms(obj);
	glm::mat4 modelViewProjectionMatrix = cam.getProjectionMatrix()*cam.getCameraMatrix()*obj.getTransform();
	glUniformMatrix4fv(uniformLocations[MODEL_VIEW_PROJECTION], 1, false, &modelViewProjectionMatrix[0][0]);
	glm::mat4 inverseRotationMatrix = cam.getRotationMatrix();
	glUniformMatrix4fv(uniformLocations[INVERSE_ROTATION_MATRIX], 1, false, &inverseRotationMatrix[0][0]);

	obj.getGeometry().drawGeometry(programID);

	glUseProgram(0);
}

void TextShader::drawVertexBinding(Camera& cam, Drawable& obj) {
	if (!programID) {
		printf("TextShader: Shader compilation failure\n");
		return;
	}

	glUseProgram(programID);

	loadMaterialUniforms(obj);
	glm::mat4 modelViewProjectionMatrix = cam.getProjectionMatrix()*cam.getCameraMatrix()*obj.getTransform();
	glUniformMatrix4fv(uniformLocations[MODEL_VIEW_PROJECTION], 1, false, &modelViewProjectionMatrix[0][0]);

	obj.getGeometry().drawGeometry(programID);

	glUseProgram(0);
}









}