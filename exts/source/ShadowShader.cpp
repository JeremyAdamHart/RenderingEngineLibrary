#include "ShadowShader.h"

namespace renderlib {

DepthVarianceShader::DepthVarianceShader() :
	ShaderT<>(
		{ {GL_VERTEX_SHADER, "shaders/depthVariance.vert"}, {GL_FRAGMENT_SHADER, "shaders/depthVariance.frag"} },
		{},
		{ "model_view_matrix", "projection_matrix" })
{}

void DepthVarianceShader::draw(Camera& cam, Drawable& obj) {
	glUseProgram(programID);

	mat4 mv_matrix = cam.getCameraMatrix()*obj.getTransform();
	mat4 projection_matrix = cam.getProjectionMatrix();

	glUniformMatrix4fv(uniformLocations[0], 1, false, &mv_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[1], 1, false, &projection_matrix[0][0]);

	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}










}