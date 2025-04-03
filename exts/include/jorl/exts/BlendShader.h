#pragma once

#include <jorl/core/Shader.h>
#include <jorl/core/Drawable.h>
#include <jorl/core/Camera.h>
#include <jorl/core/Scene.h>
#include <vector>

namespace renderlib {

class BlendShader : public Shader {
protected:
	vector<int> uniformLocations;

	void calculateUniformLocations();
public:
	BlendShader(int numSamples=0);

	//Samples if using a multisampled texture
	virtual bool createProgram(int numSamples = 0);

	void draw(Drawable &obj);
};

}