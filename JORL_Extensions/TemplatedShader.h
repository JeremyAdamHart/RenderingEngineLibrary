#pragma once

#include <Shader.h>

/***
* Templated shader class
* Make new classes inherit from this
***/

namespace renderlib {

template<typename ...Mats>
class ShaderT : public Shader {
protected:
	vector<string> materialNames;

	ShaderT(std::vector<pair<GLenum, string>> shaders,
		map<GLenum, string> defines = map<GLenum, string>{},
		vector<string> materialNames = vector<string>{}) : materialNames(materialNames)
	{
		programID = createGLProgram(shaders, defines);

		if constexpr(sizeof...(Mats) != 0)
			getMaterialOffsets<Mats...>(&materialOffsets);
		calculateUniformLocations();
	}

	virtual std::vector<std::string> getUniformNames() {
		return materialNames;
	}
	
};



};