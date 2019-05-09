#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include <assert.h>
#include <utility>

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

template<size_t S>
constexpr size_t flip() { return (S + 2) % 4; }

template<size_t S, typename Data>
struct EdgeS{
//	static_assert(Side >= 0 && Side < 4);

	std::shared_ptr<Vertex<Data>> vertex;
	std::unique_ptr<EdgeS<S, Data>> children[2];

	EdgeS<flip<S>(), Data>* pair;		//Pair is on the opposite side

	//EdgeS() :children({ nullptr, nullptr }), pair(nullptr), vertex(nullptr) {}

	EdgeS(std::shared_ptr<Vertex<Data>> vertex=nullptr, EdgeS<flip<S>(), Data>* pair = nullptr)
		: pair(pair), vertex(vertex)
	{
		if (pair)
			pair->pair = this;
	}
	EdgeS(EdgeS<flip<S>(), Data>* pair) : pair(pair), vertex(nullptr) {
		pair->pair = this;
	}

	bool hasChild() { return children[0] != nullptr && children[1] != nullptr; }
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
	static_assert(Q >= 0 && Q < 4);

	using Faces = std::tuple<
		std::unique_ptr<FaceQ<Quadrant::TL, Data>>,
		std::unique_ptr<FaceQ<Quadrant::TR, Data>>,
		std::unique_ptr<FaceQ<Quadrant::BR, Data>>,
		std::unique_ptr<FaceQ<Quadrant::BL, Data>>>;
	Faces children;

	using Edges = std::tuple<
		std::unique_ptr<EdgeS<iToS<Q, 0>(), Data>>,
		std::unique_ptr<EdgeS<iToS<Q, 1>(), Data>>,
		EdgeS<iToS<Q, 2>(), Data>*,
		EdgeS<iToS<Q, 3>(), Data>*>;
	Edges edges;



	//std::unique_ptr<Edge<Data>> insideEdges[2];		//0, 1
	//Edge<Data>* outsideEdges[2];					//2, 3
	//std::unique_ptr<FaceQ<Data>> children[4];

	FaceQ() :edges({nullptr, nullptr, nullptr, nullptr }) {}
	bool hasChild() { return children[0] != nullptr && children[1] != nullptr && children[2] != nullptr && children[3] != nullptr; }

	template<size_t C>
	FaceQ<C, Data>& child() {
		if constexpr (tuple_contains_type<std::unique_ptr<FaceQ<C, Data>>, Faces>::value)
			return *std::get<std::unique_ptr<FaceQ<C, Data>>>(children);
		else {
			static_assert(false, "child(): Child is not within range");
		}
	}

	template<size_t C>
	std::unique_ptr<FaceQ<C, Data>>& childPtr() {
		if constexpr (tuple_contains_type<std::unique_ptr<FaceQ<C, Data>>, Faces>::value)
			return std::get<std::unique_ptr<FaceQ<C, Data>>>(children);
		else {
			static_assert(false, "child(): Child is not within range");
		}
	}

	template<size_t Side>
	std::unique_ptr<EdgeS<Side, Data>>& edgeUPtr() {
		if constexpr(tuple_contains_type<std::unique_ptr<EdgeS<Side, Data>>, Edges>::value)
			return std::get<std::unique_ptr<EdgeS<Side, Data>>>(edges);
		else {
			static_assert(false, "edgeUPtr(): Edge is not stored as a unique pointer");
		}
	}

	template<size_t Side>
	EdgeS<Side, Data>*& edgePtr() {
		if constexpr(tuple_contains_type<EdgeS<Side, Data>*, Edges>::value)
			return std::get<EdgeS<Side, Data>*>(edges);
		else {
			static_assert(false, "edgePtr(): Edge is not stored as a raw pointer");
		}
	}

	template<size_t Side>
	EdgeS<Side, Data>& edge() {
		if constexpr (tuple_contains_type<std::unique_ptr<EdgeS<Side, Data>>, Edges>::value)
			return *edgeUPtr<Side>();
		else if constexpr (tuple_contains_type<EdgeS<Side, Data>*, Edges>::value)
			return *edgePtr<Side>();
		else {
			static_assert(false, "edge(): Edge not available on face");
		}
	}
};

template<typename Data>
struct TopFace {
	using Faces = std::tuple<
		std::unique_ptr<FaceQ<Quadrant::TL, Data>>,
		std::unique_ptr<FaceQ<Quadrant::TR, Data>>,
		std::unique_ptr<FaceQ<Quadrant::BR, Data>>,
		std::unique_ptr<FaceQ<Quadrant::BL, Data>>>;
	Faces children;

	using Edges = std::tuple <
		std::unique_ptr<EdgeS<Side::Top, Data>>,
		std::unique_ptr<EdgeS<Side::Left, Data>>,
		std::unique_ptr<EdgeS<Side::Right, Data>>,
		std::unique_ptr<EdgeS<Side::Bottom, Data>>>;
	Edges edges;

	TopFace() :edges({
		std::make_unique<EdgeS<Side::Top, Data>>(), 
		std::make_unique<EdgeS<Side::Left, Data>>(),
		std::make_unique<EdgeS<Side::Right, Data>>(),
		std::make_unique<EdgeS<Side::Bottom, Data>>()}) {}

	bool hasChild() { return children[0] != nullptr && children[1] != nullptr && children[2] != nullptr && children[3] != nullptr; }

	template<size_t C>
	FaceQ<C, Data>& child() {
		return *std::get<std::unique_ptr<FaceQ<C, Data>>>(children);
	}

	template<size_t C>
	std::unique_ptr<FaceQ<C, Data>>& childPtr() {
		return std::get<std::unique_ptr<FaceQ<C, Data>>>(children);
	}

	template<size_t Side>
	std::unique_ptr<EdgeS<Side, Data>>& edgeUPtr() {
		return std::get<std::unique_ptr<EdgeS<Side, Data>>>(edges);
	}

	template<size_t Side>
	EdgeS<Side, Data>& edge() {
		return *edgeUPtr<Side>();
	}
};

template<size_t S, typename Data>
void subdivideEdge(EdgeS<S, Data>& edge, 
	std::shared_ptr<Vertex<Data>> v0 = nullptr , 
	std::shared_ptr<Vertex<Data>> v1 = nullptr, 
	std::shared_ptr<Vertex<Data>> v2 = nullptr) 
{
	if (!edge.hasChild())
	{
		if(!v0) v0 = std::make_shared<Vertex<Data>>();
		if(!v1) v1 = std::make_shared<Vertex<Data>>();
		if(!v2) v2 = std::make_shared<Vertex<Data>>();

		edge.children[0] = std::make_unique<EdgeS<S, Data>>(v1);
		edge.children[1] = std::make_unique<EdgeS<S, Data>>(v2);
		edge.pair->children[0] = std::make_unique < EdgeS <flip<S>(), Data >> (v1, edge.children[1].get());
		edge.pair->children[1] = std::make_unique<EdgeS<flip<S>(), Data>>(v0, edge.children[0].get());

	}
}


template<size_t Q, typename Data>
void subdivideFace(FaceQ<Q, Data>& face) {
	using VertexPtr = std::shared_ptr<Vertex<Data>>;

	//Create child faces
	face.childPtr<Quadrant::TL>() = std::make_unique<FaceQ<Quadrant::TL, Data>>();
	face.childPtr<Quadrant::TR>() = std::make_unique<FaceQ<Quadrant::TR, Data>>();
	face.childPtr<Quadrant::BL>() = std::make_unique<FaceQ<Quadrant::BL, Data>>();
	face.childPtr<Quadrant::BR>() = std::make_unique<FaceQ<Quadrant::BR, Data>>();

	auto v_center = std::make_shared<Vertex<Data>>();

	// 00 top left, 33 bottom right
	VertexPtr vertices[3][3];

	/***********
	* Edges
	*    1__0
	*  0|    |1
	*  1|____|0 
	*    0  1
	*/

	//Subdivide outside edges
	subdivideEdge(face.edge<Side::Right>(), vertices[2][2], vertices[2][1], vertices[2][0]);
	subdivideEdge(face.edge<Side::Top>(), vertices[2][0], vertices[1][0], vertices[2][0]);
	subdivideEdge(face.edge<Side::Left>(), vertices[0][0], vertices[0][1], vertices[0][2]);
	subdivideEdge(face.edge<Side::Bottom>(), vertices[0][2], vertices[1][2], vertices[2][2]);

	face.child<Quadrant::TL>().edgePtr<Side::Top>() = face.edge<Side::Top>().children[1].get();
	face.child<Quadrant::TL>().edgePtr<Side::Left>() = face.edge<Side::Left>().children[0].get();
	face.child<Quadrant::TL>().edgeUPtr<Side::Bottom>() = std::make_unique<EdgeS<Side::Bottom, Data>>(vertices[1][1]);
	face.child<Quadrant::TL>().edgeUPtr<Side::Right>() = std::make_unique<EdgeS<Side::Right, Data>>(vertices[1][0]);

	face.child<Quadrant::TR>().edgePtr<Side::Top>() = face.edge<Side::Top>().children[0].get();
	face.child<Quadrant::TR>().edgePtr<Side::Right>() = face.edge<Side::Right>().children[1].get();
	face.child<Quadrant::TR>().edgeUPtr<Side::Bottom>() = std::make_unique<EdgeS<Side::Bottom, Data>>(vertices[2][1]);
	face.child<Quadrant::TR>().edgeUPtr<Side::Left>() = std::make_unique<EdgeS<Side::Left, Data>>(
															vertices[2][0],
															&face.child<Quadrant::TL>().edge<Side::Right>());

	face.child<Quadrant::BL>().edgePtr<Side::Bottom>() = face.edge<Side::Bottom>().children[0].get();
	face.child<Quadrant::BL>().edgePtr<Side::Left>() = face.edge<Side::Left>().children[1].get();
	face.child<Quadrant::BL>().edgeUPtr<Side::Top>() = std::make_unique<EdgeS<Side::Top, Data>>(
															vertices[0][1], 
															&face.child<Quadrant::TL>().edge<Side::Bottom>());
	face.child<Quadrant::BL>().edgeUPtr<Side::Right>() = std::make_unique<EdgeS<Side::Right, Data>>(vertices[1][1]);

	face.child<Quadrant::BR>().edgePtr<Side::Bottom>() = face.edge<Side::Bottom>().children[1].get();
	face.child<Quadrant::BR>().edgePtr<Side::Right>() = face.edge<Side::Right>().children[0].get();
	face.child<Quadrant::BR>().edgeUPtr<Side::Top>() = std::make_unique<EdgeS<Side::Top, Data>>(
															vertices[1][1],
															&face.child<Quadrant::TR>().edge<Side::Bottom>());
	face.child<Quadrant::BR>().edgeUPtr<Side::Left>() = std::make_unique<EdgeS<Side::Left, Data>>(
															vertices[1][2],
															&face.child<Quadrant::BL>().edge<Side::Right>());
}

template<typename Data>
void subdivideFace(TopFace<Data>& face) {
	using VertexPtr = std::shared_ptr<Vertex<Data>>;

	//Create child faces
	face.childPtr<Quadrant::TL>() = std::make_unique<FaceQ<Quadrant::TL, Data>>();
	face.childPtr<Quadrant::TR>() = std::make_unique<FaceQ<Quadrant::TR, Data>>();
	face.childPtr<Quadrant::BL>() = std::make_unique<FaceQ<Quadrant::BL, Data>>();
	face.childPtr<Quadrant::BR>() = std::make_unique<FaceQ<Quadrant::BR, Data>>();

	auto v_center = std::make_shared<Vertex<Data>>();

	// 00 top left, 33 bottom right
	VertexPtr vertices[3][3];

	/***********
	*    1__0
	*  0|    |1
	*  1|____|0
	*    0  1
	************/

	//Subdivide outside edges
	subdivideEdge(face.edge<Side::Right>(), vertices[2][2], vertices[2][1], vertices[2][0]);
	subdivideEdge(face.edge<Side::Top>(), vertices[2][0], vertices[1][0], vertices[2][0]);
	subdivideEdge(face.edge<Side::Left>(), vertices[0][0], vertices[0][1], vertices[0][2]);
	subdivideEdge(face.edge<Side::Bottom>(), vertices[0][2], vertices[1][2], vertices[2][2]);

	face.child<Quadrant::TL>().edgePtr<Side::Top>() = face.edge<Side::Top>().children[1].get();
	face.child<Quadrant::TL>().edgePtr<Side::Left>() = face.edge<Side::Left>().children[0].get();
	face.child<Quadrant::TL>().edgeUPtr<Side::Bottom>() = std::make_unique<EdgeS<Side::Bottom, Data>>(vertices[1][1]);
	face.child<Quadrant::TL>().edgeUPtr<Side::Right>() = std::make_unique<EdgeS<Side::Right, Data>>(vertices[1][0]);

	face.child<Quadrant::TR>().edgePtr<Side::Top>() = face.edge<Side::Top>().children[0].get();
	face.child<Quadrant::TR>().edgePtr<Side::Right>() = face.edge<Side::Right>().children[1].get();
	face.child<Quadrant::TR>().edgeUPtr<Side::Bottom>() = std::make_unique<EdgeS<Side::Bottom, Data>>(vertices[2][1]);
	face.child<Quadrant::TR>().edgeUPtr<Side::Left>() = std::make_unique<EdgeS<Side::Left, Data>>(
		vertices[2][0],
		&face.child<Quadrant::TL>().edge<Side::Right>());

	face.child<Quadrant::BL>().edgePtr<Side::Bottom>() = face.edge<Side::Bottom>().children[0].get();
	face.child<Quadrant::BL>().edgePtr<Side::Left>() = face.edge<Side::Left>().children[1].get();
	face.child<Quadrant::BL>().edgeUPtr<Side::Top>() = std::make_unique<EdgeS<Side::Top, Data>>(
		vertices[0][1],
		&face.child<Quadrant::TL>().edge<Side::Bottom>());
	face.child<Quadrant::BL>().edgeUPtr<Side::Right>() = std::make_unique<EdgeS<Side::Right, Data>>(vertices[1][1]);

	face.child<Quadrant::BR>().edgePtr<Side::Bottom>() = face.edge<Side::Bottom>().children[1].get();
	face.child<Quadrant::BR>().edgePtr<Side::Right>() = face.edge<Side::Right>().children[0].get();
	face.child<Quadrant::BR>().edgeUPtr<Side::Top>() = std::make_unique<EdgeS<Side::Top, Data>>(
		vertices[1][1],
		&face.child<Quadrant::TR>().edge<Side::Bottom>());
	face.child<Quadrant::BR>().edgeUPtr<Side::Left>() = std::make_unique<EdgeS<Side::Left, Data>>(
		vertices[1][2],
		&face.child<Quadrant::BL>().edge<Side::Right>());
}

template<typename T>
class VariableSizeGrid {
	int m_minX;
	int m_minY;
	std::vector<std::vector<T>> m_data;

public:
	VariableSizeGrid() :m_minX(0), m_minY(0) {}
	template<typename ...Args>
	VariableSizeGrid(size_t dim_x, size_t dim_y, Args... args) : m_minX(0), m_minY(0), m_data(dim_y, std::vector<T>(dim_x, args...)) {}

	size_t xSize() { return m_data[0].size(); }
	size_t ySize() { return m_data.size(); }

	T& operator()(int y, int x) {
		//return m_data[(y - m_minY)*xSize() + x - m_minX];
		return m_data[y - m_minY][x - m_minX];
	}

	const T& operator()(int y, int x) const {
		//return m_data[(y - m_minY)*xSize() + x - m_minX];
		return m_data[y - m_minY][x - m_minX];
	}

};

template<typename Data>
std::vector<TopFace<Data>> initializeTopFaceGrid(size_t xFaces, size_t yFaces) {
	VariableSizeGrid<TopFace<Data>> faceGrid (xFaces, yFaces);
	VariableSizeGrid<std::shared_ptr<Vertex<Data>>> vertexGrid(xFaces + 1, yFaces + 1);

	for (int y = 0; y < yFaces; y++) {
		for (int x = 0; x < xFaces; x++) {
			faceGrid(y, x).edge<Side::Top>().vertex = vertexGrid(y, x);
			faceGrid(y, x).edge<Side::Left>().vertex = vertexGrid(y+1, x);
			faceGrid(y, x).edge<Side::Bottom>().vertex = vertexGrid(y+1, x+1);
			faceGrid(y, x).edge<Side::Right>().vertex = vertexGrid(y, x+1);

			if (x > 0) {
				faceGrid(y, x).edge<Side::Left>().pair = &faceGrid(y, x-1).edge<Side::Right>();
				faceGrid(y, x-1).edge<Side::Right>().pair = &faceGrid(y, x).edge<Side::Left>();
			}
			if (y < 0) {
				faceGrid(y, x).edge<Side::Top>().pair = &faceGrid(y-1, x).edge<Side::Bottom>();
				faceGrid(y-1, x).edge<Side::Bottom>().pair = &faceGrid(y, x).edge<Side::Top>();
			}
		}
	}

}

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
