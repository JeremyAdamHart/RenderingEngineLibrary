#include "BlendShader.h"
#include "TextureMat.h"
#include "glSupport.h"

using namespace glm;

namespace renderlib {

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/blend.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/blend.frag" }
};

BlendShader::BlendShader(int numSamples) {
	createProgram(numSamples);
	calculateUniformLocations();
}

bool BlendShader::createProgram(int numSamples) {
	if (numSamples)
		programID = createGLProgram(shaders,
		{ {GL_FRAGMENT_SHADER, str("#define NUM_SAMPLES %d\n", numSamples)} });
	else
		programID = createGLProgram(shaders);

	return programID;
}

void BlendShader::calculateUniformLocations() {
	glUseProgram(programID);

	//Material uniforms
	uniformLocations.resize(TextureMat::COUNT*2);
	uniformLocations[TextureMat::TEXTURE_LOCATION] = 
		glGetUniformLocation(programID, "opaqueTexture");
	uniformLocations[TextureMat::TEXTURE_LOCATION + TextureMat::COUNT] = 
		glGetUniformLocation(programID, "translucentTexture");
}

void BlendShader::draw(Drawable &obj) {
	glUseProgram(programID);
	obj.loadUniforms(TextureMat::id, &uniformLocations[0]);
	obj.loadUniforms(TextureMat::id | TextureMat::TRANSLUCENT, 
		&uniformLocations[TextureMat::COUNT]);

	obj.getGeometry().drawGeometry(programID);
	glUseProgram(0);
}

}
