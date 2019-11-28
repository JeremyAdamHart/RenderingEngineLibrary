#pragma once

#include "GLObject.h"
#include "glSupport.h"
#include "Drawable.h"

namespace renderlib {

class Shader {
protected:
	GLProgram programID;
	vector<int> uniformLocations;
	std::map<int, int> materialOffsets;

	Shader();
	Shader(std::map<int, int> materialOffsets);
	Shader(int materialID);

public:
	GLProgram getID();
	void deleteProgram();
	virtual bool createProgram(map<GLenum, string> defines = map<GLenum, string>{});

	virtual std::vector<std::string> getUniformNames();

	virtual void calculateUniformLocations();

	virtual void loadMaterialUniforms(const Drawable &drawable);
};

template<typename T1, typename T2, typename ...Ts>
void getMaterialOffsets(std::map<int, int>* offsetMap, int totalOffset = 0) {
	getMaterialOffsets<T1>(offsetMap, totalOffset);
	getMaterialOffsets<T2, Ts...>(offsetMap, totalOffset + T1::COUNT);
}



template<typename T>
void getMaterialOffsets(std::map<int, int>* offsetMap, int totalOffset = 0) {
	(*offsetMap)[T::id] = totalOffset;
}

}