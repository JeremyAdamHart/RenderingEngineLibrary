#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include <assert.h>
#include <utility>
#include <random>
#include <ctime>

namespace adaptive {


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
	std::shared_ptr<Vertex<Data>> child;
	Vertex() :child(nullptr) {}
};

/*
*     2
*    ___
* 3 |   | 1
*   |___|
*     0
*/
enum Side : size_t {
	Bottom=0,
	Right,
	Top,
	Left
};


template<size_t S>
constexpr size_t flip() { return (S + 2) % 4; }

template<size_t S, typename Data>
struct EdgeS{
//	static_assert(Side >= 0 && Side < 4);

	std::shared_ptr<Vertex<Data>> vertex;
	std::unique_ptr<EdgeS<S, Data>> children[2];

	EdgeS<flip<S>(), Data>* pair;		//Pair is on the opposite side

	EdgeS(std::shared_ptr<Vertex<Data>> vertex=nullptr, EdgeS<flip<S>(), Data>* pair = nullptr)
		: pair(pair), vertex(vertex), children{nullptr, nullptr }
	{
		if (pair)
			pair->pair = this;
	}
	EdgeS(EdgeS<flip<S>(), Data>* pair) : pair(pair), vertex(nullptr) {
		pair->pair = this;
	}

	bool hasChild() { return children[0] && children[1]; }
};

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
template<size_t S>
static constexpr size_t sToV() { return (6 - S) % 4; }					//Side to vertex
template<size_t V>
static constexpr size_t vToS() { return (6 - V) % 4; }

//Consider subclassing off of Face<Data>
template<size_t Q, typename Data>
struct FaceQ{
	static_assert(Q >= 0 && Q < 4);
	using DataType = Data;

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

	FaceQ() :edges({
		std::make_unique<EdgeS<iToS<Q, 0>(), Data>>(), 
		std::make_unique<EdgeS<iToS<Q, 1>(), Data>>(), 
		nullptr, nullptr }) 
	{}

	bool hasChild() { return std::get<std::unique_ptr<FaceQ<Quadrant::TL, Data>>>(children) != nullptr; }

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

	template<size_t V>
	Vertex<Data>& vertex() {
		return *edge<vToS<V>()>().vertex;
	}

	template<size_t V>
	std::shared_ptr<Vertex<Data>>& vertexPtr() {
		return edge<vToS<V>() > ().vertex;
	}
};

template<typename Data>
struct TopFace {
	using DataType = Data;

	using Faces = std::tuple<
		std::unique_ptr<FaceQ<Quadrant::TL, Data>>,
		std::unique_ptr<FaceQ<Quadrant::TR, Data>>,
		std::unique_ptr<FaceQ<Quadrant::BR, Data>>,
		std::unique_ptr<FaceQ<Quadrant::BL, Data>>>;
	Faces children;

	using Edges = std::tuple <
		std::unique_ptr<EdgeS<Side::Bottom, Data>>,
		std::unique_ptr<EdgeS<Side::Right, Data>>,
		std::unique_ptr<EdgeS<Side::Top, Data>>,
		std::unique_ptr<EdgeS<Side::Left, Data>>>;
	Edges edges;

	TopFace() :edges({
		std::make_unique<EdgeS<Side::Bottom, Data>>(), 
		std::make_unique<EdgeS<Side::Right, Data>>(),
		std::make_unique<EdgeS<Side::Top, Data>>(),
		std::make_unique<EdgeS<Side::Left, Data>>()}) {}

	bool hasChild() { return std::get<0>(children) && std::get<1>(children) && std::get<2>(children) && std::get<3>(children); }

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

	template<size_t Q>
	Vertex<Data>& vertex() {
		return *edge<vToS<Q>()>().vertex;
	}

	template<size_t Q>
	std::shared_ptr<Vertex<Data>>& vertexPtr() {
		return edge<vToS<Q>()>().vertex;
	}
};

template<size_t S, typename Data>
void subdivideEdge(EdgeS<S, Data>& edge, Vertex<Data>& p0, Vertex<Data>& p1){
	if (!edge.hasChild())
	{
		auto v0 = p0.child = (p0.child) ? p0.child : std::make_shared<Vertex<Data>>();
		auto v1 = std::make_shared<Vertex<Data>>();
		auto v2 = p1.child = (p1.child) ? p1.child : std::make_shared<Vertex<Data>>();

		edge.children[0] = std::make_unique<EdgeS<S, Data>>(v1);
		edge.children[1] = std::make_unique<EdgeS<S, Data>>(v2);
		if (edge.pair) {
			edge.pair->children[0] = std::make_unique < EdgeS <flip<S>(), Data >>(v1, edge.children[1].get());
			edge.pair->children[1] = std::make_unique<EdgeS<flip<S>(), Data>>(v0, edge.children[0].get());
		}
	}
}

template<size_t V, typename Face_t>
bool hasGeneratedCorner(Face_t& face) {
	//return face.edge<vToS<V>()>().hasChild() || face.edge<(vToS<V>()+1)%4>().hasChild();
	return face.vertex<V>().child != nullptr;
}

template<size_t V, typename Face_t>
std::shared_ptr<Vertex<typename Face_t::DataType>> getGeneratedCorner(Face_t& face) {
	/*if (face.edge<vToS<V>()>().hasChild())
		return face.edge<vToS<V>()>().children[1]->vertex;
	else if (face.edge<(vToS<V>() + 1) % 4>().hasChild())
		return face.edge<(vToS<V>() + 1) % 4>().children[0]->pair->vertex;
	else
		return nullptr;*/
	return face.vertex<V>().child;
}

template<size_t V, typename Face_t>
std::shared_ptr<Vertex<typename Face_t::DataType>>& childCornerPtr(Face_t& face) {
	if (face.edge<vToS<V>()>().hasChild()) {
		return face.edge<vToS<V>()>().children[1]->vertex;
	}
	else if (face.edge<(vToS<V>() + 1) % 4>().hasChild()) {
		return face.edge<(vToS<V>() + 1) % 4>().children[0]->pair->vertex;
	}
	else
		return std::make_shared<Vertex<typename Face_t::DataType>>();
}

template<typename Face_t, typename Data>
void subdivideFaceImp(Face_t& face) {
	using VertexPtr = std::shared_ptr<Vertex<Data>>;

	bool test = hasGeneratedCorner<Quadrant::TL, Face_t>(face);

	//Create child faces
	face.childPtr<Quadrant::TL>() = std::make_unique<FaceQ<Quadrant::TL, Data>>();
	face.childPtr<Quadrant::TR>() = std::make_unique<FaceQ<Quadrant::TR, Data>>();
	face.childPtr<Quadrant::BL>() = std::make_unique<FaceQ<Quadrant::BL, Data>>();
	face.childPtr<Quadrant::BR>() = std::make_unique<FaceQ<Quadrant::BR, Data>>();

	auto v_center = std::make_shared<Vertex<Data>>();

	/***********
	* Edges
	*    1__0
	*  0|    |1
	*  1|____|0
	*    0  1
	*/

	//Subdivide outside edges
	subdivideEdge(face.edge<Side::Right>(), face.vertex<Quadrant::BR>(), face.vertex<Quadrant::TR>());
	subdivideEdge(face.edge<Side::Top>(), face.vertex<Quadrant::TR>(), face.vertex<Quadrant::TL>());
	subdivideEdge(face.edge<Side::Left>(), face.vertex<Quadrant::TL>(), face.vertex<Quadrant::BL>());
	subdivideEdge(face.edge<Side::Bottom>(), face.vertex<Quadrant::BL>(), face.vertex<Quadrant::BR>());

	face.child<Quadrant::TL>().edgePtr<Side::Top>() = face.edge<Side::Top>().children[1].get();
	face.child<Quadrant::TL>().edgePtr<Side::Left>() = face.edge<Side::Left>().children[0].get();
	face.child<Quadrant::TL>().edgeUPtr<Side::Bottom>() = std::make_unique<EdgeS<Side::Bottom, Data>>(v_center);
	face.child<Quadrant::TL>().edgeUPtr<Side::Right>() = std::make_unique<EdgeS<Side::Right, Data>>(
		face.edge<Side::Top>().children[0]->vertex);	//vertices[1][0]);

	face.child<Quadrant::TR>().edgePtr<Side::Top>() = face.edge<Side::Top>().children[0].get();
	face.child<Quadrant::TR>().edgePtr<Side::Right>() = face.edge<Side::Right>().children[1].get();
	face.child<Quadrant::TR>().edgeUPtr<Side::Bottom>() = std::make_unique<EdgeS<Side::Bottom, Data>>(
		face.edge<Side::Right>().children[0]->vertex);	//vertices[2][1]);
	face.child<Quadrant::TR>().edgeUPtr<Side::Left>() = std::make_unique<EdgeS<Side::Left, Data>>(
		v_center,
		&face.child<Quadrant::TL>().edge<Side::Right>());

	face.child<Quadrant::BL>().edgePtr<Side::Bottom>() = face.edge<Side::Bottom>().children[0].get();
	face.child<Quadrant::BL>().edgePtr<Side::Left>() = face.edge<Side::Left>().children[1].get();
	face.child<Quadrant::BL>().edgeUPtr<Side::Top>() = std::make_unique<EdgeS<Side::Top, Data>>(
		face.edge<Side::Left>().children[0]->vertex, //vertices[0][1],
		&face.child<Quadrant::TL>().edge<Side::Bottom>());
	face.child<Quadrant::BL>().edgeUPtr<Side::Right>() = std::make_unique<EdgeS<Side::Right, Data>>(v_center);

	face.child<Quadrant::BR>().edgePtr<Side::Bottom>() = face.edge<Side::Bottom>().children[1].get();
	face.child<Quadrant::BR>().edgePtr<Side::Right>() = face.edge<Side::Right>().children[0].get();
	face.child<Quadrant::BR>().edgeUPtr<Side::Top>() = std::make_unique<EdgeS<Side::Top, Data>>(
		v_center,
		&face.child<Quadrant::TR>().edge<Side::Bottom>());
	face.child<Quadrant::BR>().edgeUPtr<Side::Left>() = std::make_unique<EdgeS<Side::Left, Data>>(
		face.edge<Side::Bottom>().children[0]->vertex,	//vertices[1][2],
		&face.child<Quadrant::BL>().edge<Side::Right>());
}

template<size_t Q, typename Data>
void subdivideFace(FaceQ<Q, Data>& face) {
	subdivideFaceImp<FaceQ<Q, Data>, Data>(face);
}

template<typename Data>
void subdivideFace(TopFace<Data>& face) {
	subdivideFaceImp<TopFace<Data>, Data>(face);
}

template<typename T>
class VariableSizeGrid {
	int m_minX;
	int m_minY;
	std::vector<std::vector<T>> m_data;

public:
	VariableSizeGrid() :m_minX(0), m_minY(0) {}
	template<typename ...Args>
	VariableSizeGrid(size_t dim_x, size_t dim_y, Args... args) : m_minX(0), m_minY(0) //, m_data(dim_y, std::vector<T>(dim_x, args...)) 
	{
		for (size_t i = 0; i < dim_y; i++) {
			m_data.push_back(std::vector<T> ());
			for (size_t j = 0; j < dim_x; j++) {
				m_data[i].emplace_back(std::forward<T>(args)...);
			}
		}
	}

	size_t xSize() { return m_data[0].size(); }
	size_t ySize() { return m_data.size(); }

	T& operator()(int y, int x) {
		return m_data[y - m_minY][x - m_minX];
	}

	const T& operator()(int y, int x) const {
		return m_data[y - m_minY][x - m_minX];
	}

};

template<typename Data>
VariableSizeGrid<TopFace<Data>> initializeTopFaceGrid(size_t xFaces, size_t yFaces) {
	VariableSizeGrid<TopFace<Data>> faceGrid (xFaces, yFaces);
	VariableSizeGrid<std::shared_ptr<Vertex<Data>>> vertexGrid(xFaces + 1, yFaces + 1);
	for (int y = 0; y < vertexGrid.ySize(); y++) {
		for (int x = 0; x < vertexGrid.xSize(); x++) {
			vertexGrid(y, x) = std::make_shared<Vertex<Data>>();
		}
	}

	for (int y = 0; y < yFaces; y++) {
		for (int x = 0; x < xFaces; x++) {
			faceGrid(y, x).edge<Side::Left>().vertex = vertexGrid(y, x);
			faceGrid(y, x).edge<Side::Top>().vertex = vertexGrid(y+1, x);
			faceGrid(y, x).edge<Side::Right>().vertex = vertexGrid(y+1, x+1);
			faceGrid(y, x).edge<Side::Bottom>().vertex = vertexGrid(y, x+1);

			if (x > 0) {
				faceGrid(y, x).edge<Side::Left>().pair = &faceGrid(y, x-1).edge<Side::Right>();
				faceGrid(y, x-1).edge<Side::Right>().pair = &faceGrid(y, x).edge<Side::Left>();
			}
			if (y > 0) {
				faceGrid(y, x).edge<Side::Bottom>().pair = &faceGrid(y-1, x).edge<Side::Top>();
				faceGrid(y-1, x).edge<Side::Top>().pair = &faceGrid(y, x).edge<Side::Bottom>();
			}
		}
	}

	return faceGrid;
}

float randNorm();

class Noise {
public:
	float value;
	float value2;

	Noise() :value(randNorm()*2.f - 1.f), value2(randNorm()*2.f - 1.f) {}
	glm::vec2 vec() { return glm::vec2(value, value2); }
};

template<typename Face>
float evaluate(Face& face, glm::vec2 p) {
	float v0 = (1 - p.x)*face.vertex<Quadrant::BL>().d.value + p.x*face.vertex<Quadrant::BR>().d.value;
	float v1 = (1 - p.x)*face.vertex<Quadrant::TL>().d.value + p.x*face.vertex<Quadrant::TR>().d.value;

	return (1 - p.y)*v0 + p.y*v1;
}

inline float smoothStep(float t) {
	return 6.f*t*t*t*t*t - 15.f*t*t*t*t + 10.f*t*t*t;
}

inline float norm1(glm::vec2 v) { return std::max(abs(v.x), abs(v.y)); }

inline float evaluatePerlinSingle(glm::vec2 noiseVector, glm::vec2 p) {
	return (norm1(p) < 1.0f) ?
		dot(noiseVector, p)*smoothStep(1.f - abs(p.x))*smoothStep(1.f - abs(p.y)) :
		0.f;
}

template<typename Face>
float evaluatePerlin(Face& face, glm::vec2 p) {
	glm::vec2 blVec = p;
	glm::vec2 tlVec = p - glm::vec2(0.f, 1.f);
	glm::vec2 brVec = p - glm::vec2(1.f, 0.f);
	glm::vec2 trVec = p - glm::vec2(1.f, 1.f);

	float v0 = smoothStep(1 - p.x)*dot(face.vertex<Quadrant::BL>().d.vec(), blVec) + smoothStep(p.x)*dot(face.vertex<Quadrant::BR>().d.vec(), brVec);
	float v1 = smoothStep(1 - p.x)*dot(face.vertex<Quadrant::TL>().d.vec(), tlVec) + smoothStep(p.x)*dot(face.vertex<Quadrant::TR>().d.vec(), trVec);

	return smoothStep(1 - p.y)*v0 + smoothStep(p.y)*v1;
}

template<typename Face_t>
float evaluateClosest(Face_t& face, glm::vec2 p) {
	if (p.x > 0.5f) {
		if (p.y > 0.5f)
			return face.vertex<Quadrant::TR>().d.value;
		else
			return face.vertex<Quadrant::BR>().d.value;
	}
	else {
		if (p.y > 0.5f)
			return face.vertex<Quadrant::TL>().d.value;
		else
			return face.vertex<Quadrant::BL>().d.value;
	}
}

template<size_t S>
float distFromEdge(glm::vec2 p) {
	if constexpr (S == Side::Left)
		return p.x;
	else if constexpr (S == Side::Right)
		return 1.f - p.x;
	if constexpr (S == Side::Bottom)
		return p.y;
	else if constexpr (S == Side::Top)
		return 1.f - p.y;
	else {
		static_assert(false, "distFromEdge(): Invalid edge");
		return 0.f;
	}
}

template<size_t Q>
glm::vec3 vecFromCorner(glm::vec2 p) {
	if constexpr (Q == Quadrant::BL)
		return p;
	else if constexpr (Q == Quadrant::BR)
		return p - glm::vec2(1.f, 0.f);
	else if constexpr (Q == Quadrant::TL)
		return p - glm::vec2(0.f, 1.f);
	else if constexpr (Q == Quadrant::TR)
		return p - glm::vec2(0.f, 1.f);
	else constexpr{
		static_assert(false, "vecFromCorner(): Invalid corner");
		return glm::vec2(0.f);
	}
}

template<typename Face_t, size_t S>
float evaluateSide(Face_t& face, glm::vec2 p) {
	float u = distFromEdge<S>(p);
}

template<typename Face_t>
float evaluatePartialFace(Face_t& face, glm::vec2 p) {
	float sum = 0.f;
	glm::vec2 p_bl = 2.f*p;
	glm::vec2 p_br = 2.f*(p - glm::vec2(1, 0));
	glm::vec2 p_tl = 2.f*(p - glm::vec2(0, 1));
	glm::vec2 p_tr = 2.f*(p - glm::vec2(1, 1));

	if (face.edge<Side::Left>().hasChild())
		sum += evaluatePerlinSingle(face.edge<Side::Left>().children[0]->vertex->d.vec(), 0.5f*(p_bl + p_tl));
	if (face.edge<Side::Top>().hasChild())
		sum += evaluatePerlinSingle(face.edge<Side::Top>().children[0]->vertex->d.vec(), 0.5f*(p_tl + p_tr));
	if (face.edge<Side::Right>().hasChild())
		sum += evaluatePerlinSingle(face.edge<Side::Right>().children[0]->vertex->d.vec(), 0.5f*(p_br + p_tr));
	if (face.edge<Side::Bottom>().hasChild())
		sum += evaluatePerlinSingle(face.edge<Side::Bottom>().children[0]->vertex->d.vec(), 0.5f*(p_bl + p_br));

	if(hasGeneratedCorner<Quadrant::BL>(face))
		sum += evaluatePerlinSingle(getGeneratedCorner<Quadrant::BL>(face)->d.vec(), p_bl);
	if (hasGeneratedCorner<Quadrant::TL>(face))
		sum += evaluatePerlinSingle(getGeneratedCorner<Quadrant::TL>(face)->d.vec(), p_tl);
	if (hasGeneratedCorner<Quadrant::BR>(face))
		sum += evaluatePerlinSingle(getGeneratedCorner<Quadrant::BR>(face)->d.vec(), p_br);
	if (hasGeneratedCorner<Quadrant::TR>(face))
		sum += evaluatePerlinSingle(getGeneratedCorner<Quadrant::TR>(face)->d.vec(), p_tr);

	return sum;
}


class SimpleNoiseField{
public:
	VariableSizeGrid<TopFace<Noise>> noise;
	SimpleNoiseField(int width, int height);
	
	template<typename Face>
	float evaluateAtImp(Face& face, glm::vec2 point, glm::vec2 bottomLeft, glm::vec2 dim, float factor) {
		glm::vec2 normalizedPoint = (point - bottomLeft) / dim;
		float result = evaluatePerlin(face, normalizedPoint)*factor;

		if (face.hasChild()) {
			if (point.x - bottomLeft.x > dim.x*0.5f) {
				bottomLeft.x += dim.x*0.5f;
				if (point.y - bottomLeft.y > dim.y*0.5f) {
					bottomLeft.y += dim.y*0.5f;
					return result + evaluateAtImp<FaceQ<Quadrant::TR, Noise>>(face.child<Quadrant::TR>(), point, bottomLeft, dim*0.5f, factor*0.5f);
				}
				else
					return result + evaluateAtImp(face.child<Quadrant::BR>(), point, bottomLeft, dim*0.5f, factor*0.5f);
			}
			else {
				if (point.y - bottomLeft.y > dim.y*0.5f) {
					bottomLeft.y += dim.y*0.5f;
					return result + evaluateAtImp(face.child<Quadrant::TL>(), point, bottomLeft, dim*0.5f, factor*0.5f);
				}
				else
					return result + evaluateAtImp(face.child<Quadrant::BL>(), point, bottomLeft, dim*0.5f, factor*0.5f);
			}
		}
		else
			return result +evaluatePartialFace(face, normalizedPoint)*factor*0.5f;
	}

	float evaluateAt(glm::vec2 point);

	void subdivideSquare(glm::vec2 point);

	template<typename Face>
	void subdivideSquareImp(Face& face, glm::vec2 point, glm::vec2 bottomLeft, glm::vec2 dim) {
		if (face.hasChild()) {
			if (point.x - bottomLeft.x > dim.x*0.5f) {
				bottomLeft.x += dim.x*0.5f;
				if (point.y - bottomLeft.y > dim.y*0.5f) {
					bottomLeft.y += dim.y*0.5f;
					subdivideSquareImp(face.child <Quadrant::TR>(), point, bottomLeft, dim*0.5f);
				}
				else
					subdivideSquareImp(face.child<Quadrant::BR>(), point, bottomLeft, dim*0.5f);
			}
			else {
				if (point.y - bottomLeft.y > dim.y*0.5f) {
					bottomLeft.y += dim.y*0.5f;
					return subdivideSquareImp(face.child<Quadrant::TL>(), point, bottomLeft, dim*0.5f);
				}
				else
					return subdivideSquareImp(face.child<Quadrant::BL>(), point, bottomLeft, dim*0.5f);
			}
		}
		else {
			subdivideFace(face);
		}
	}
};

}
