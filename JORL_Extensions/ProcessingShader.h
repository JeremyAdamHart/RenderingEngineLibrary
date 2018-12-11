#pragma once

#include "Shader.h"
#include "Drawable.h"
#include "Scene.h"
#include <vector>
#include <glm/glm.hpp>

namespace renderlib {

class ProcessingShader : public Shader {
	vector<pair<GLenum, string>> shaders;
public:
	ProcessingShader(string fragmentShader, string defines=string());

	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});

	void draw(Drawable &obj);		//Change to use pointer to drawable?
	void draw(Scene &scene);
};

}