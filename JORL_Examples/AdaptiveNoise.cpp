#include "AdaptiveNoise.h"

namespace adaptive {


 float randNorm(){
	return float(rand()) / float(RAND_MAX);
}


SimpleNoiseField::SimpleNoiseField(int width, int height)  {
	noise = initializeTopFaceGrid<Noise>(width, height);
}

float SimpleNoiseField::evaluateAt(glm::vec2 point) {
	float cellWidth = 1.f / float(noise.xSize());
	float cellHeight = 1.f / float(noise.ySize());

	int indexX = glm::clamp(int(point.x / cellWidth), 0, int(noise.xSize()-1));
	int indexY = glm::clamp(int(point.y / cellHeight), 0, int(noise.ySize()-1));

	return 0.5f + 0.5f*evaluateAtImp(noise(indexY, indexX), point, 
		glm::vec2(	float(indexX)*cellWidth, 
					float(indexY)*cellHeight), 
		glm::vec2(cellWidth, cellHeight), 1.f);
}

void SimpleNoiseField::subdivideSquare(glm::vec2 point) {
	float cellWidth = 1.f / float(noise.xSize());
	float cellHeight = 1.f / float(noise.ySize());

	int indexX = glm::clamp(int(point.x / cellWidth), 0, int(noise.xSize() - 1));
	int indexY = glm::clamp(int(point.y / cellHeight), 0, int(noise.ySize() - 1));

	subdivideSquareImp(noise(indexY, indexX), point,
		glm::vec2(float(indexX)*cellWidth,
			float(indexY)*cellHeight),
		glm::vec2(cellWidth, cellHeight));
}



}
