#include "noiseTest.h"

using namespace adaptive;

static int count = 0;

Counter::Counter() :val(count++) {}

void validateTopFaces(VariableSizeGrid<TopFace<Noise>>& grid) {
	printf("////////////////////////\n//Checking top faces\n////////////////////////\n");
	for (int y = 0; y < grid.ySize() - 1; y++) {
		for (int x = 0; x < grid.xSize() - 1; x++) {
			if (grid(y, x).edge<Side::Right>().pair != &grid(y, x + 1).edge<Side::Left>())
				printf("grid(%d, %d)<Right> pair incorrect\n", y, x);
			if (grid(y, x + 1).edge<Side::Left>().pair != &grid(y, x).edge<Side::Right>())
				printf("grid(%d, %d)<Left> pair incorrect\n", y, x + 1);
			if (grid(y, x).edge<Side::Top>().pair != &grid(y + 1, x).edge<Side::Bottom>())
				printf("grid(%d, %d)<Top> pair incorrect\n", y, x);
			if (grid(y + 1, x).edge<Side::Bottom>().pair != &grid(y, x).edge<Side::Top>())
				printf("grid(%d, %d)<Bottom> pair incorrect\n", y + 1, x);

			if (grid(y, x).vertex<Quadrant::TR>().d.value != grid(y, x + 1).vertex<Quadrant::TL>().d.value)
				printf("grid(%d, %d)<TR> != grid(%d, %d)<TL>\n", y, x, y, x + 1);
			if (grid(y, x).vertex<Quadrant::BR>().d.value != grid(y, x + 1).vertex<Quadrant::BL>().d.value)
				printf("grid(%d, %d)<BR> != grid(%d, %d)<BL>\n", y, x, y, x + 1);

			if (grid(y, x).vertex<Quadrant::TL>().d.value != grid(y + 1, x).vertex<Quadrant::BL>().d.value)
				printf("grid(%d, %d)<TL> != grid(%d, %d)<BL>\n", y, x, y + 1, x);
			if (grid(y, x).vertex<Quadrant::TR>().d.value != grid(y + 1, x).vertex<Quadrant::BR>().d.value)
				printf("grid(%d, %d)<TR> != grid(%d, %d)<BR>\n", y, x, y + 1, x);

			if (grid(y, x).vertex<Quadrant::TR>().d.value != grid(y + 1, x + 1).vertex<Quadrant::BL>().d.value)
				printf("grid(%d, %d)<TR> != grid(%d, %d)<BL>\n", y, x, y + 1, x + 1);
		}
	}
}

template<typename Face_t>
std::vector<int> getFaceVertices(Face_t& face) {
	////////////
	//  0---1
	//  |   |
	//  2---3
	////////////

	std::vector<int> vec ={
		face.vertex<Quadrant::TL>().d.val,
		face.vertex<Quadrant::TR>().d.val,
		face.vertex<Quadrant::BL>().d.val,
		face.vertex<Quadrant::BR>().d.val
	};

	return vec;
}

template<typename Face_t>
void validateSubdivideFace(Face_t& face) {
	printf("////////////////////////\n//Checking subdivide faces\n////////////////////////\n");

	subdivideFace(face);

	auto tlFace = getFaceVertices(face.child<Quadrant::TL>());
	auto trFace = getFaceVertices(face.child<Quadrant::TR>());
	auto blFace = getFaceVertices(face.child<Quadrant::BL>());
	auto brFace = getFaceVertices(face.child<Quadrant::BR>());

	printf("%d %d | %d %d\n%d %d | %d %d\n---------\n%d %d | %d %d\n%d %d | %d %d\n",
		tlFace[0], tlFace[1], trFace[0], trFace[1], tlFace[2], tlFace[3], trFace[2], trFace[3],
		blFace[0], blFace[1], brFace[0], brFace[1], blFace[2], blFace[3], brFace[2], brFace[3]);

	return;
}


void runAdaptiveNoiseTests() {
	SimpleNoiseField simpNoise(2, 2);
	validateTopFaces(simpNoise.noise);

	count = 0;
	TopFace<Counter> face;
	face.vertexPtr<Quadrant::TL>() = std::make_shared<Vertex<Counter>>();
	face.vertexPtr<Quadrant::TR>() = std::make_shared<Vertex<Counter>>();
	face.vertexPtr<Quadrant::BL>() = std::make_shared<Vertex<Counter>>();
	face.vertexPtr<Quadrant::BR>() = std::make_shared<Vertex<Counter>>();

	printf("===Top Face===\n");
	validateSubdivideFace<TopFace<Counter>>(face);

	count = 0;
	printf("===Face TL===\n");
	validateSubdivideFace(face.child<Quadrant::TL>());

	//count = 0;
	printf("===Face TR===\n");
	validateSubdivideFace(face.child<Quadrant::TR>());

	//count = 0;
	printf("===Face BL===\n");
	validateSubdivideFace(face.child<Quadrant::BL>());

	//count = 0;
	printf("===Face BR===\n");
	validateSubdivideFace(face.child<Quadrant::BR>());
}