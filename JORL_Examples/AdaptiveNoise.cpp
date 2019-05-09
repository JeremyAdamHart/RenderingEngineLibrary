#include "AdaptiveNoise.h"

 float randNorm(){
	return float(rand()) / float(RAND_MAX);
}


SimpleNoiseField::SimpleNoiseField(int width, int height) :noise(width, height) {}
/*
float SimpleNoiseField::evaluateAt(glm::vec2 point) {
	float cellWidth = 1.f / float(noise.xSize());
	float cellHeight = 1.f / float(noise.ySize());

	int indexX = int(point.x / cellWidth);
	int indexY = int(point.y / cellHeight);

	return evaluateAt(noise(indexX, indexY), point, 
		glm::vec2(float(indexX)*cellWidth, 
		float(indexY)*cellHeight), 
		glm::vec2(cellWidth, cellHeight));
}
*/