#pragma once

#include "TemplatedShader.h"
#include "ColorMat.h"
#include "TextureMat.h"
#include "Camera.h"
#include "Framebuffer.h"

namespace renderlib {
class FlatColorShader : public ShaderT<ColorMat> {
public:
	FlatColorShader();
	void draw(Camera& cam, Drawable obj);
};

class GaussianBlurShader : public ShaderT<TextureMat>{
public:
	enum class Direction : int {
		X=0,
		Y
	};

	GaussianBlurShader();
	//std::vector<std::string> getUniformNames() override;
	void draw(float sigma, int n, Direction direction, Drawable& obj);
};


}