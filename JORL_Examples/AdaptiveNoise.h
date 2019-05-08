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

	Edge(std::shared_ptr<Vertex<Data>> vertex, Edge<Data>* pair = nullptr) 
		:child(nullptr), next(nullptr), pair(pair), vertex(vertex) 
	{
		if (pair)
			pair->pair = this;
	}
	Edge(Edge<Data>* pair) {
		pair->pair = this;
	}

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
	std::unique_ptr<Face<Data>> children [4];

	Face() :edges({ nullptr, nullptr, nullptr, nullptr }), next(nullptr), children({ nullptr, nullptr, nullptr, nullptr }) {}
	bool hasChild() { return children[0] != nullptr && children[1] == nullptr && children[2] == nullptr && children[3] == nullptr; }

	Edge& edge(size_t side) {
		return edges[side];
	}
};

//Add additional template for quadrant to avoid assert statements?
template<typename Data>
struct Face {
	size_t quadrant;
	std::unique_ptr<Edge<Data>> insideEdges[2];		//0, 1
	Edge<Data>* outsideEdges[2];					//2, 3
	std::unique_ptr<Face<Data>> children [4];

	Face(size_t quadrant) :quadrant(quadrant), edges({ nullptr, nullptr, nullptr, nullptr }), next(nullptr), children({ nullptr, nullptr, nullptr, nullptr }) {}
	bool hasChild() { return children[0] != nullptr && children[1] != nullptr && children[2] != nullptr && children[3] != nullptr; }

	size_t sideToIndex(size_t side) { return (side + quadrant) % 4; }
	size_t indexToSide(size_t index) { return (side + 4 - quadrant) % 4; }

	Edge& edge(size_t side) {
		size_t index = sideToIndex(side);
		if (index < 2)
			return *insideEdges[index];
		else
			return outsideEdges[index - 2];
	}

	std::unique_ptr<Edge<Data>>& insideEdgePtr(size_t side) {
		size_t index = sideToIndex(side);
		assert(index < 2);
		return insideEdges[index];
	}

	Edge<Data>* outsideEdgePtr(size_t side) {
		size_t index = sideToIndex(side);
		assert(index > 1 && index < 4);
		return outsideEdges[index];
	}
};

template<typename Data, size_t Q>
struct FaceQ{
	static_assert(Q >= 0 && Q < 4);
	constexpr size_t sToI(size_t side) { return (side + Q) % 4; }			//Side to index
	constexpr size_t iToS(size_t index) { return (index + 4 - Q) % 4; }		//Index to side

	template<typename Ptr, size_t Side>
	struct EdgeSide {
		Ptr edgePtr;

		Edge& get() { return *edgePtr; }
		//Set function?
	};

	std::tuple<
		EdgeSide<std::unique_ptr, iToS(0)>,
		std::unique_ptr<FaceQ<Data, iToS(1)>>,
		Face<Data, iToS(2)>*,
		Face<Data, iToS(3)>*> faces;

	std::unique_ptr<Edge<Data>> insideEdges[2];		//0, 1
	Edge<Data>* outsideEdges[2];					//2, 3
	std::unique_ptr<FaceQ<Data>> children[4];

	FaceQ(size_t quadrant) :quadrant(quadrant), edges({ nullptr, nullptr, nullptr, nullptr }), next(nullptr), children({ nullptr, nullptr, nullptr, nullptr }) {}
	bool hasChild() { return children[0] != nullptr && children[1] != nullptr && children[2] != nullptr && children[3] != nullptr; }


	Edge& edge(size_t side) {
		size_t index = sideToIndex(side);
		if (index < 2)
			return *insideEdges[index];
		else
			return outsideEdges[index - 2];
	}

	std::unique_ptr<Edge<Data>>& insideEdgePtr(size_t side) {
		size_t index = sideToIndex(side);
		assert(index < 2);
		return insideEdges[index];
	}

	Edge<Data>* outsideEdgePtr(size_t side) {
		size_t index = sideToIndex(side);
		assert(index > 1 && index < 4);
		return outsideEdges[index];
	}
};

template<typename Data>
void subdivideEdge(Edge<Data>& edge) {
	if (!edge.hasChild())
	{
		auto v0 = std::make_shared<Vertex<Data>>();
		auto v1 = std::make_shared<Vertex<Data>>();
		auto v2 = std::make_shared<Vertex<Data>>();

		edge.children[0] = std::make_unique<Edge<Data>>(v1);
		edge.children[1] = std::make_unique<Edge<Data>>(v2);
		edge.pair.children[0] = std::make_unique<Edge<Data>>(v1, edge.children[1].get());
		edge.pair.children[0] = std::make_unique<Edge<Data>>(v0, edge.children[0].get());
		
	}
}

template<typename Data>
void subdivideFace(TopFace<Data>& face) {

}

template<typename Data>
void subdivideFace(Face<Data>& face) {
	for(size_t i=0; i<3; i++)
		face[i] = std::make_unique<Face<Data>>(i);

	auto v_center = std::make_shared<Vertex<Data>>();
	
	//Subdivide existing edges
	for (int i = 0; i < 4; i++) 
		subdivideEdge(face.edge(i));

	
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