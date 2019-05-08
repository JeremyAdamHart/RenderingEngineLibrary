#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include <assert.h>

//Taken from answer:
// https://stackoverflow.com/questions/25958259/how-do-i-find-out-if-a-tuple-contains-a-type
//vv from Piotr Skotnicki
template <typename T, typename Tuple>
struct has_type;

template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

template <typename T, typename Tuple>
using tuple_contains_type = typename has_type<T, Tuple>::type;
//^^ Piotr Skotnicki

template<typename Data>
struct Vertex {
	Data d;
};
/*
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
};*/

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
/*
template<typename Data>
struct TopFace {
	std::unique_ptr<Edge<Data>> edges[4];
	std::unique_ptr<Face<Data>> next;
	std::unique_ptr<Face<Data>> children [4];

	Face() :edges({ nullptr, nullptr, nullptr, nullptr }), next(nullptr), children({ nullptr, nullptr, nullptr, nullptr }) {}
	bool hasChild() { return children[0] != nullptr && children[1] == nullptr && children[2] == nullptr && children[3] == nullptr; }

	Edge<Data>& edge(size_t side) {
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
*/
template<size_t Side, typename Data>
struct EdgeS{
//	static_assert(Side >= 0 && Side < 4);

	std::shared_ptr<Vertex<Data>> vertex;
	std::unique_ptr<EdgeS<Side, Data>> children[2];

	EdgeS<(Side + 2) % 4, Data>* pair;		//Pair is on the opposite side
};

/*template<size_t X, size_t Side, typename Data, typename ...Args>
std::unique_ptr<EdgeS<X, Data>> getSide(std::tuple < std::unique_ptr<EdgeS<Side, Data>, Args...>& list) {
	static_assert(std::is_same<X, Side>::value);
	return get_tuple
}*/


enum Quadrant : size_t {
	TL = 0,
	TR,
	BR,
	BL
};

template<size_t Q, size_t Side>
static constexpr size_t sToI() { return (Side + Q) % 4; }			//Side to index
template<size_t Q, size_t Index>
static constexpr size_t iToS() { return (Index + 4 - Q) % 4; }		//Index to side

template<size_t Q, typename Data>
struct FaceQ{
//	static_assert(Q >= 0 && Q < 4);

	/*template<typename Ptr, size_t Side>
	struct EdgeSide {
		Ptr edgePtr;

		Edge& get() { return *edgePtr; }
		//Set function?
	};*/

	/*std::tuple<
		EdgeSide<std::unique_ptr<Edge<Data>>, iToS(0)>,
		EdgeSide<std::unique_ptr<Edge<Data>> iToS(1)>>,
		Face<Data, iToS(2)>*,
		Face<Data, iToS(3)>*> faces;*/

	using Edges = std::tuple<
		std::unique_ptr<EdgeS<iToS<Q, 0>(), Data>>,
		std::unique_ptr<EdgeS<iToS<Q, 1>(), Data>>,
		EdgeS<iToS<Q, 2>(), Data>*,
		EdgeS<iToS<Q, 3>(), Data>*>;
	Edges edges;



	//std::unique_ptr<Edge<Data>> insideEdges[2];		//0, 1
	//Edge<Data>* outsideEdges[2];					//2, 3
	//std::unique_ptr<FaceQ<Data>> children[4];

	FaceQ() :edges({ nullptr, nullptr, nullptr, nullptr }) {}
	bool hasChild() { return children[0] != nullptr && children[1] != nullptr && children[2] != nullptr && children[3] != nullptr; }

	template<size_t Side>
	std::unique_ptr<EdgeS<Side, Data>>& edgeUPtr() {
		return get<std::unique_ptr<EdgeS<Side, Data>>&(edges);
	}

	template<size_t Side>
	EdgeS<Side, Data>* edgePtr() {
		return get<EdgeS<Side, Data>*(edges);
	}

	template<size_t Side>
	EdgeS<Side, Data>& edge() {
		if constexpr (tuple_contains_type<std::unique_ptr<EdgeS<Side, Data>>, Edges>::value) {
			//return *edgeUPtr<Side>();
		} if constexpr (tuple_contains_type<EdgeS<Side, Data>*, Edges>::value) {
			//static_assert(false, "EdgeS<Side, Data>& edge(): Edge not available on face");
			//return *edgePtr<Side>();
		}
	}

	/*
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
	*/
};
/*
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
*/

/*
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
*/
