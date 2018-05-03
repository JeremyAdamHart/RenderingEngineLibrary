#pragma once

#include "glSupport.h"
#include "Drawable.h"

namespace renderlib {

class Shader {
protected:
	GLuint programID;
	vector<int> uniformLocations;
	std::map<int, int> materialOffsets;

	Shader();
	Shader(std::map<int, int> materialOffsets);
	Shader(int materialID);
//	Shader(vector<int> materialIDs);

public:
	GLuint getID();
	void deleteProgram();
	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});

	//New functions
	virtual std::vector<std::string> getUniformNames();

	virtual void calculateUniformLocations();

	virtual void loadMaterialUniforms(const Drawable &drawable);
};

}