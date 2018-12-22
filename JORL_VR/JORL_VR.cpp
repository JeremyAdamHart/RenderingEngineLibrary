// OpenVRTest.cpp : Defines the entry point for the console application.
//

#include "VRWindow.h"
#include <string>
#include <iostream>

#include "ConvexHull.h"


int main(int argc, char** argv)
{
	SlotMap<int> map;

	SlotMap<int>::Index a = map.add(1);
	SlotMap<int>::Index b = map.add(2);
	SlotMap<int>::Index c = map.add(3);
	map.remove(b);
	SlotMap<int>::Index d = map.add(4);
	SlotMap<int>::Index e = map.add(5);

	for (auto it = map.begin(); it != map.end(); ++it) {
		std::cout << (*it) << std::endl;
	}
	/*
	HalfEdgeMesh<glm::vec3> mesh;
	generateTetrahedron(mesh, glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));

	std::vector<glm::vec3> points;
	std::vector<int> indices;
	halfEdgeToFaceList(&points, &indices, mesh);
	*/

	WindowManager wm(800, 400, "VR Segmenting");
	char* loadFilename = "icosahedron.ply";	//"untrackedmodels/riccoSurface_take3.obj";	//"models/dragon.obj";
	char* saveFilename = "saved/default.clr";
	int multisampling = 8;
	switch (argc) {
	case 1:
		//Change to ricco
		loadFilename = "untrackedmodels/Craspedia2.ply";	// "untrackedmodels/Helianthus4.ply";	//"untrackedmodels/GRCD2RNA.ply";	//"models/Cube.obj";	//"models/icosahedron.ply";
		saveFilename = "saved/default.clr";
		break;
	case 2:
		loadFilename = argv[1];
		saveFilename = argv[1];
		break;
	case 3:
		loadFilename = argv[1];
		saveFilename = argv[2];
		break;
	case 4:
		loadFilename = argv[1];
		saveFilename = argv[2];
		multisampling = std::stoi(argv[3]);
	}

	wm.paintingLoop(loadFilename, saveFilename, multisampling);
}