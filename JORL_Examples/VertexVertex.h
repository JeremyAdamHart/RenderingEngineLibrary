#pragma once

#include "glmSupport.h"
#include <vector>
 
namespace renderlib {
namespace vv {


class VertexAdjacencyBuilder {
public:
	std::vector<std::vector<unsigned int>> neighbourChains;

	void addVertices(unsigned int v0, unsigned int v1);
};

class Adjacency{
public:
	std::vector<unsigned int> neighbours;

	void removeDuplicates();
};

template<typename T>
std::vector<Adjacency> faceListToVertexVertex(std::vector<unsigned int>& faces, std::vector<T>& vertices) {
	std::vector<unsigned > outputVertices;

	std::vector<VertexAdjacencyBuilder> adjacency;
	adjacency.resize(vertices.size());

	for (int i = 0; i+2 < faces.size(); i += 3) {
		unsigned int a = faces[i];
		unsigned int b = faces[i + 1];
		unsigned int c = faces[i + 2];

		auto a_adj = &adjacency[faces[i]];
		auto b_adj = &adjacency[faces[i + 1]];
		auto c_adj = &adjacency[faces[i + 2]];

		a_adj->addVertices(b, c);
		b_adj->addVertices(c, a);
		c_adj->addVertices(a, b);
	}

	std::vector<Adjacency> outputAdjacency;
	outputAdjacency.resize(adjacency.size());

	for (int i = 0; i < vertices.size(); i++) {
		outputAdjacency[i].neighbours = adjacency[i].neighbourChains.front();
		if (outputAdjacency[i].neighbours.front() == outputAdjacency[i].neighbours.back())
			outputAdjacency[i].neighbours.pop_back();
	}

	return outputAdjacency;
}

}
}