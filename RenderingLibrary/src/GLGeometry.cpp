#include "GLGeometry.h"

using namespace std;

namespace renderlib {

//GWW's answer: https://stackoverflow.com/questions/4157687/using-char-as-a-key-in-stdmap?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
bool stringMapCompare::operator()(const char* a, const char* b) {
	return strcmp(a, b) < 0;
}

VertexAttribLayout::VertexAttribLayout(GLuint program, vector<const char*> attributeNames){
	for (const char* attrib : attributeNames) {
		attribMap[attrib] = glGetAttribLocation(program, attrib);
	}
}

bool VertexAttribLayout::operator==(const VertexAttribLayout& b) const{
	return attribMap == b.attribMap;
}

bool VertexAttribLayout::operator!=(const VertexAttribLayout& b) const{
	return !(attribMap == b.attribMap);
}

int& VertexAttribLayout::operator[](const char* attribName) {
	return attribMap[attribName];
}

const int &VertexAttribLayout::operator[](const char* attribName) const{
	return attribMap.at(attribName);
}

bool VertexAttribLayout::isComplete() const{
	for (auto keyValuePair : attribMap) {
		if (keyValuePair.second < 0)
			return false;
	}

	return true;
}

VertexArraySelector::VertexArraySelector() :nextMapValue(1){}

GLuint VertexArraySelector::getVAO(GLuint program) {
	vaoMap[programMap[program]];
}

bool VertexArraySelector::isKnownProgram(GLuint program) {
	return programMap.find(program) != programMap.end();
}

bool VertexArraySelector::registerLayout(GLuint program, const VertexAttribLayout& layout) {
	for (auto it : layouts) {
		if (it.second == layout) {
			programMap[program] = it.first;
			return true;
		}
	}

	programMap[program] = nextMapValue;
	layouts[nextMapValue] = layout;
	vaoMap[nextMapValue] = 0;		//No valid VAO
	nextMapValue++;
	return false;
}

bool VertexArraySelector::hasValidVAO(GLuint program) {
	return vaoMap[programMap[program]] != 0;
}

void VertexArraySelector::registerVAO(GLuint program, GLuint vao) {
	vaoMap[programMap[program]] = vao;
}



}