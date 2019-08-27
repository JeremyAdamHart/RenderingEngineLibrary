#pragma once

#include "glmSupport.h"
#include <vector>
 
namespace renderlib {
namespace vv {

template<typename T>
class VertexAdjacencyBuilder {
public:
	std::vector<std::vector<Vertex<T>*>> neighbourChains;

	VertexAdjacencyBuilder() {}

	void addVertices(Vertex<T>* v0, Vertex<T*> v1) {
		int insertedAtFront = -1;
		int insertedAtBack = -1;
		int index = 0;
		for (auto chain : neighbourChains) {
			if (v0 == chain.back()) {
				chain.push_back(v1);
				insertedAtBack = index;
			}
			else if (v1 == chain.front()) {
				chain.insert(chain.begin(), v0);
				insertedAtFront = index;
			}
			index++;
		}
		if (insertedAtFront < 0 && insertedAtBack < 0) {
			chain.push_back({ v0, v1 });
			return;
		}
		//If pair conected two chains
		else if (insertedAtFront > 0 && insertedAtBack > 0) {
			auto& frontChain = neighbourChains[insertedAtBack];
			auto& backChain = neighbourChains[insertedAtFront];
			frontChain.pop_back();		//Remove duplicated element at back
			frontChain.insert(frontChain.end(), backChain.begin(), backChain.end());
		}
	}
};

template<typename T>
class Vertex{
public:
	T v;
	std::vector<Vertex<T>*> neighbours;

	Vertex(T v) :v(v) {}
	
};


template<typename T>
std::vector<Vertex<T>> faceListToVertexVertex(std::vector<unsigned int>& faces, std::vector<T>& vertices) {
	std::vector<Vertex<T>> outputVertices;
	
	for (auto v : vertices)
		outputVertices.push_back(Vertex<T>(v));

	std::vector<VertexAdjacencyBuilder> adjacency;
	adjacency.resize(outputVertices.size());

	for (int i = 0; i+2 < faces.size(); i += 3) {
		Vertex<T>* a = &outputVertices[faces[i]];
		Vertex<T>* b = &outputVertices[faces[i + 1]];
		Vertex<T>* c = &outputVertices[faces[i + 2]];

		auto a_adj = &adjacency[faces[i]];
		auto b_adj = &adjacency[faces[i + 1]];
		auto c_adj = &adjacency[faces[i + 2]];

		a_adj->addVertices(b, c);
		b_adj->addVertices(c, a);
		c_adj->addVertices(a, b);
	}

	for (int i = 0; i < vertices.size(); i++)
		outputVertices[i].neighbours = adjacency[i].neighbourChains.front();
}

}
}