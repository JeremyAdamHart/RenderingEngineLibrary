#pragma once
#include "TemplatedShader.h"
#include "TextureMat.h"
#include "ColorMat.h"
#include "Camera.h"
#include "Drawable.h"


namespace renderlib {


class TextShader : public ShaderT<TextureMat, ColorMat> {
public:
	TextShader();
	void draw(Camera& cam, Drawable& obj);
	void drawVertexBinding(Camera& cam, Drawable& obj);
};

class SimpleTextShader : public ShaderT<TextureMat, ColorMat> {
public:
	SimpleTextShader();
	void draw(Camera& cam, Drawable& obj);
	void drawVertexBinding(Camera& cam, Drawable& obj);
};

}