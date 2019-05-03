#pragma once

#include "Quadtree.h"
#include <glm/glm.hpp>
#include <vector>

template<typename Data>
struct Vertex {
	Data d;
};

template<typename Data>
struct Edge {
	std::unique_ptr<Edge<Data>> children[2];
	Edge<Data>* next;
	Edge<Data>* pair;
	std::shared_ptr<Vertex<Data>> vertex;

	Edge(std::shared_ptr<Vertex<Data>> vertex = nullptr, Edge<Data>* pair = nullptr) :child(nullptr), next(nullptr), pair(pair), vertex(vertex) {}
	bool hasChild() { return children[0] != nullptr && children[0] != nullptr; }
};

/*
*     0
*    ___
* 3 |   | 1
*   |___|
*     2
*/
enum Side : size_t {
	Bottom=0,
	Right,
	Top,
	Left
};

template<typename Data>
struct TopFace {
	std::unique_ptr<Edge<Data>> edges[4];
	std::unique_ptr<Face<Data>> next;
	std::unique_ptr<Face<Data>> child;

	Face() :edges({ nullptr, nullptr, nullptr, nullptr }), next(nullptr), child(nullptr) {}
	bool hasChild() { return child != nullptr; }
};

template<typename Data>
struct Face {
	size_t quadrant;
	std::unique_ptr<Edge<Data>> insideEdges[2];		//0, 1
	Edge<Data>* outsideEdges[2];					//2, 3
	std::unique_ptr<Face<Data>> next;
	std::unique_ptr<Face<Data>> child;

	Face() :edges({ nullptr, nullptr, nullptr, nullptr }), next(nullptr), child(nullptr) {}
	bool hasChild() { return child != nullptr; }

	size_t sideToIndex(size_t side) { return (side + quadrant) % 4; }
	size_t indexToSide(size_t index) { return (side + 4 - quadrant) % 4; }

	Edge& edge(size_t side) {
		size_t index = sideToIndex(side);
		if (index < 2)
			insideEdges[index];
		else
			outsideEdges[index - 2];
	}
};

template<typename Data>
void subdivideEdge(Edge<Data>& edge) {
	if (edge.hasChild()) {
		
	} else {
		edge.children[0] = new Edge<Data>();
		edge.children[1] =  new Edge<Data>());
		edge.children[0].next = edge.children[1];
		
	}
}

template<typename Data>
void subdivideFace(Face<Data>& face) {

}

class Octave {
	float amplitude;

};

class Noise2D {
	int width, height;
	glm::vec2 topLeftCorner;
	float topLevelWidth;

	std::vector<float> amplitudes;
	std::vector<std::unique_ptr<Quadtree<glm::vec2>>> noise;
public:
	Noise2D(int width, int height, glm::vec2 topLeftCorner, float topLevelWidth);
	


};