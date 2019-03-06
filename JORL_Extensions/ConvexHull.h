#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>

//Container which reserves space in middle
template<typename T>
class SlotMap {

	std::vector<T> data;
	std::vector<int> timestamp;
	std::vector<int> emptySlots;
	size_t dataSize;

public:
	SlotMap() :dataSize(0) {}

/*	struct IndexData {
		int index;
		int timestamp;
	};
	*/
	struct Index {
		int index;
		int timestamp;

		Index() :index(-1), timestamp(0) {}
	//	Index(IndexData id) :index(id.index), timestamp(id.timestamp) {}
		Index(int index) :index(index), timestamp(0) {}
		Index(int index, int timestamp) :index(index), timestamp(timestamp) {}
		operator int() const {return index; }
//		operator IndexData() const{ return {.index = index,.timestamp = timestamp}; }
		operator size_t() const { return index; }
		operator bool() { return index >= 0; }
		bool operator==(Index other) const { return other.index == index && other.timestamp == timestamp; }
		bool operator!=(Index other) const { return !((*this) == other); }
		bool operator<(Index other) const { return index < other.index; }
		bool operator>(Index other) const { return index > other.index; }
		bool operator<=(Index other) const { return index <= other.index; }
		bool operator>=(Index other) const { return index >= other.index; }
	};

	Index add(T value) {
		dataSize++;
		if (emptySlots.empty()) {
			data.push_back(value);
			timestamp.push_back(0);
			return Index(data.size()-1);
		}
		else {
			int index = emptySlots.back();
			data[index] = value;
			timestamp[index]++;
			emptySlots.pop_back();
			return Index(index, timestamp[index]);
		}
	}

	bool valid(Index i) {
		return (i.index >= 0) && (i.index < timestamp.size()) && (timestamp[i.index] == i.timestamp);
	}

	void remove(Index i) {
		emptySlots.push_back(i.index);
		timestamp[i.index]++;
		dataSize = (dataSize == 0) ? dataSize : dataSize-1;
	}

	T& operator[](Index i) {
		if (i.index == -1 || i.index >= timestamp.size())
 			throw std::invalid_argument("Out of bounds");
		if (timestamp[i.index] != i.timestamp)
			throw std::invalid_argument("Timestamp expired");
		return data[i.index];
	}

	const T& operator[](Index i) const {
		if (i.index == -1 || i.index >= timestamp.size())
			throw std::invalid_argument("Out of bounds");
		if (timestamp[i.index] != i.timestamp)
			throw std::invalid_argument("Timestamp expired");
		return data[i.index];
	}

	Index random() const {
		int i;
		do {
			i = rand() % timestamp.size();
		} while (timestamp[i] % 2);
		return{ i, timestamp[i] };
	}

	/*const T& operator[] const (Index i) {
		return data[i];
	}*/

	int size() { return dataSize; }

	//Iterator
	class Iterator {
		int index;
		SlotMap<T>* container;

		friend SlotMap<T>;

		Iterator(int index, SlotMap<T>* container) :index(index), container(container) {}
	public:
		Iterator() :index(-1), container(nullptr) {}

		Iterator& operator++() {
			do {
				index++;
			} while (index < container->data.size() && (container->timestamp[index] % 2) == 1);

			if (index >= container->data.size())
				*this = container->end();
			return *this;
//			return (index < container->data.size()) ? *this : container->end();
		}

		Iterator& operator--() {
			do {
				index--;
			} while (index > 0 && (container->timestamp[index] % 2) == 1);

			if (index < 0)
				*this = container->begin();
			return *this;
		}

		//operator Index() const { return{ index, timestamp[index] }; }

		T& operator*() { return container->data[index]; }
		T* operator->() { return &container->data[index]; }

		bool operator==(const Iterator& it) const { return index == it.index && container == it.container; }
		bool operator!=(const Iterator& it) const { return !(*this == it); }
		bool operator<(const Iterator& it) const { return index < it.index; }
		bool operator>(const Iterator& it) const { return index > it.index; }
		bool operator<=(const Iterator& it) const { return index <= it.index; }
		bool operator>=(const Iterator& it) const { return index >= it.index; }

		Index toIndex() const { return{ index, container->timestamp[index] }; }
		//T operator
	};

	Iterator begin() {
		if (data.size() == 0) return Iterator(0, this);

		int i = 0;
		while (i < data.size() && (timestamp[i] % 2) == 1) {
			i++;
		}
		return (i < data.size()) ? Iterator(i, this)  : Iterator(0, this);
	}

	Iterator end() {
		if (data.size() == 0) return Iterator(0, this);

		int i = data.size() - 1;
		while (i > 0 && (timestamp[i] % 2) == 1) {
			i--;
		}
		return {i+1, this };
	}
};


template<typename P> class Face;
template<typename P> class Vertex;

template<typename P>
struct HalfEdge {
	typename SlotMap<Vertex<P>>::Index head;
	typename SlotMap<HalfEdge<P>>::Index next;
	typename SlotMap<HalfEdge<P>>::Index pair;
	typename SlotMap<Face<P>>::Index face;

	bool operator==(const HalfEdge<P>& other) const { 
		return head == other.head && next == other.next && pair == other.pair && face == other.face; 
	}
};

template<typename P>
struct Vertex {
	P pos;
	typename SlotMap<HalfEdge<P>>::Index edge;

	bool operator==(const Vertex<P>& other) const { 
		return pos == other.pos && edge 
	}
};

template<typename P>
struct Face {
	typename SlotMap<HalfEdge<P>>::Index edge;

	bool operator==(const Face<P>& other) const {
		return edge == other.edge;
	}
};


//template<typename P>
//using HalfEdgeIndex = SlotMap<HalfEdge<P>>::Index;
//template<typename P>
//using VertexIndex = SlotMap<Vertex<P>>::Index;
//template<typename P>
//using FaceIndex = SlotMap<Face<P>>::Index;


template<typename P>
class HalfEdgeMesh {	
public:
	SlotMap<Vertex<P>> vertices;
	SlotMap<Face<P>> faces;
	SlotMap<HalfEdge<P>> edges;

	//Accessor methods
	HalfEdge<P>& next(HalfEdge<P> edge) { return edges[edge.next]; }
	const HalfEdge<P>& next(HalfEdge<P> edge) const { return edges[edge.next]; }
	HalfEdge<P>& pair(HalfEdge<P> edge) { return edges[edge.pair]; }
	const HalfEdge<P>& pair(HalfEdge<P> edge) const { return edges[edge.pair]; }
	Vertex<P>& head(HalfEdge<P> edge) { return vertices[edge.head]; }
	const Vertex<P>& head(HalfEdge<P> edge) const { return vertices[edge.head]; }
	Face<P>& face(HalfEdge<P> edge) { return vertices[edge.face]; }
	const Face<P>& face(HalfEdge<P> edge) const { return vertices[edge.face]; }

	HalfEdge<P>& edge(Vertex<P> vert) { return edges[vert.edge]; }
	const HalfEdge<P>& edge(Vertex<P> vert) const { return edges[vert.edge]; }
	HalfEdge<P>& edge(Face<P> face) { return edges[face.edge]; }
	const HalfEdge<P>& edge(Face<P> face) const { return edges[face.edge]; }

	const HalfEdge<P>& operator[](typename SlotMap<HalfEdge<P>>::Index index) const { return edges[index]; }
	HalfEdge<P>& operator[](typename SlotMap<HalfEdge<P>>::Index index) { return edges[index]; }

	const Vertex<P>& operator[](typename SlotMap<Vertex<P>>::Index index) const { return vertices[index]; }
	Vertex<P>& operator[](typename SlotMap<Vertex<P>>::Index index) { return vertices[index]; }

	const Face<P>& operator[](typename SlotMap<Face<P>>::Index index) const { return faces[index]; }
	Face<P>& operator[](typename SlotMap<Face<P>>::Index index) { return faces[index]; }

	bool isBoundary(typename SlotMap<HalfEdge<P>>::Index edge) { return !edges[edge].face; }

	typename SlotMap<HalfEdge<P>>::Index prev(typename SlotMap<HalfEdge<P>>::Index start) {
		SlotMap<HalfEdge<P>>::Index current = start;
		while (edges[current].next != start)
			current = edges[current].next;

		return current;
	}

	//Follows around the vertex until the halfedge is found which either has start as next->pair, or no next
	typename SlotMap<HalfEdge<P>>::Index circleVertex(typename SlotMap<HalfEdge<P>>::Index start) {
		SlotMap<HalfEdge<P>>::Index current = start;
		while (edges[current].next && edges[edges[current].next].pair != start)
			current = edges[edges[current].next].pair;

		return current;
	}

	//Deprecated function
	typename SlotMap<HalfEdge<P>>::Index nextOnBoundary(typename SlotMap<HalfEdge<P>>::Index start) {
		SlotMap<HalfEdge<P>>::Index e = edges[start].next;
		while (!isBoundary(edges[e].pair) && edges[e].pair != start) 
			e = edges[edges[e].pair].next;

		return (edges[e].pair == start) ? SlotMap<HalfEdge<P>>::Index() : e;
		
	}

	P normal(Face<P> f) {
		P a = head(edge(f)).pos;
		P b = head(next(edge(f))).pos;
		P c = head(next(next(edge(f)))).pos;

		return normalize(cross(b - a, c - a));
	}

	//Only supports 3 point faces currently
	P center(Face<P> f) {
		P a = head(edge(f)).pos;
		P b = head(next(edge(f))).pos;
		P c = head(next(next(edge(f)))).pos;

		return (a + b + c) / 3.f;
	}

	//Returns half edge on other side of new boundary -- REVISED VERSION returns half edge on boundary
	typename SlotMap<HalfEdge<P>>::Index deleteFace(typename SlotMap<Face<P>>::Index f) {
		
		/*
		//NO HALF EDGES ON BOUNDARY VERSION
		SlotMap<HalfEdge<P>>::Index returnedEdge;

		SlotMap<HalfEdge<P>>::Index e = edges[faces[f].edge].next;
		while (e != faces[f].edge) {
			if (edges[e].pair) {
				returnedEdge = edges[e].pair;
				pair(edges[e]).pair = {};
			}

			//Can speed up with half-edges on boundary
			auto endIterator = edges.end();
			for (SlotMap<HalfEdge<P>>::Iterator edge = edges.begin(); edge != edges.end(); ++edge) {
				if (edge.toIndex() != e && edge->head == edges[e].head)
					vertices[edge->head].edge = edge.toIndex();
			}

			SlotMap<HalfEdge<P>>::Index temp = e;
			e = edges[e].next;
			edges.remove(temp);
		}

		if (edges[e].pair) {
			pair(edges[e]).pair = {};
		}

		edges.remove(e);
		faces.remove(f);
	
		return returnedEdge;
		*/
		
		//HALF EDGES ON BOUNDARY VERSION
		SlotMap<HalfEdge<P>>::Index lastLeadingEdge;

		std::vector <SlotMap<HalfEdge<P>>::Index> edgeList = { faces[f].edge };
		do {
			edgeList.push_back(edges[edgeList.back()].next);
		} while (edgeList.back() != faces[f].edge);

		edgeList.pop_back();

		SlotMap<HalfEdge<P>>::Index returnedEdge;

		//SlotMap<HalfEdge<P>>::Index e = faces[f].edge;
		for(SlotMap<HalfEdge<P>>::Index e : edgeList){
			SlotMap<HalfEdge<P>>::Index e_pair = edges[e].pair;
			if (isBoundary(e_pair)) {

				SlotMap<HalfEdge<P>>::Index e_prev = prev(e);
				SlotMap<HalfEdge<P>>::Index e_pair_next = edges[e_pair].next;
				SlotMap<HalfEdge<P>>::Index e_pair_prev = prev(e_pair);
				SlotMap<HalfEdge<P>>::Index e_next = edges[e].next;
				edges[e_prev].next = e_pair_next;
				edges[e_pair_prev].next = e_next;

				//Reassign vertex edges if they would be removed
				SlotMap<Vertex<P>>::Index vert = edges[e].head;
				if (vertices[vert].edge = e) {
					SlotMap<HalfEdge<P>>::Index newEdge = edges[edges[e].next].pair;
					if (newEdge == e)
						vertices.remove(vert);		//Might not be robust?
					else
						vertices[vert].edge = newEdge;
				}

				edges.remove(e_pair);
				edges.remove(e);

				//e = e_next;	//Increment to next edge
			}
			else {
				returnedEdge = e;
				edges[e].face = SlotMap<Face<P>>::Index();

				//e = edges[e].next;	//Increment to next edge
			}

		}	// while (e != faces[f].edge);

		faces.remove(f);

		if (!edges.valid(returnedEdge))
			printf("Invalid edge returned\n");
		return returnedEdge;
		
	}
};

template<typename P, typename I>
void halfEdgeToFaceList(std::vector<P>* vertices, std::vector<I>* indices, HalfEdgeMesh<P>& mesh) {

	I count = vertices->size();
	std::map<SlotMap<Vertex<P>>::Index, I> indexMap;
	for (SlotMap<Vertex<P>>::Iterator vert = mesh.vertices.begin(); vert != mesh.vertices.end(); ++vert) {
		vertices->push_back(vert->pos);
		indexMap[vert.toIndex()] = count++;
	}
	
	
	for (typename SlotMap<Face<P>>::Iterator face = mesh.faces.begin(); 
		face != mesh.faces.end(); 
		++face) 
	{
		//Revisit --> Need to track the halfedge, not the vertex, vertIndex should be edgeIndex
		HalfEdge<P> edge = mesh.edge(*face);
		indices->push_back(indexMap[edge.head]);
		edge = mesh.next(edge);	
		indices->push_back(indexMap[edge.head]);
		edge = mesh.next(edge);
		indices->push_back(indexMap[edge.head]);

		//auto edgeIndex = mesh.edge(*face).head;
		//indices->push_back(indexMap[vertindex]);
		//vertindex = mesh.next(mesh.edge(mesh[vertindex])).head;
		//indices->push_back(indexMap[vertindex]);
		//vertindex = mesh.next(mesh.edge(mesh[vertindex])).head;
		//indices->push_back(indexMap[vertindex]);
	}
}

//P is vertex type, I is index type
template <typename P, typename I>
typename SlotMap<Vertex<P>>::Index faceListToHalfEdge(HalfEdgeMesh<P>* mesh, const std::vector<P>& vertices, const std::vector<I>& indices) {
	std::vector<SlotMap<Vertex<P>>::Index> vertIndices;

	struct VertexPair {
		I a, b;
		VertexPair(I a, I b) :a(a), b(b) {}
		bool operator<(const VertexPair& other) const { return (a == other.a) ? b < other.b : a < other.a; }
	};

	//Add all vertices
	for (int i = 0; i < vertices.size(); i++)
		vertIndices.push_back(mesh->vertices.add({ vertices[i], {} }));

	//Maps vertex pair to corresponding half edge
	std::map <VertexPair, SlotMap<HalfEdge<P>>::Index> edgeMap;

	//Convert faces
	for (int i = 0; i+2 < indices.size(); i+=3) {

		auto face = mesh->faces.add({});

		SlotMap<HalfEdge<P>>::Index edges[3];

		for (int j = 0; j < 3; j++) {
			I a = indices[i + j];
			I b = indices[i + (j + 1) % 3];

			SlotMap<Vertex<P>>::Index v_a = vertIndices[a];
			SlotMap<Vertex<P>>::Index v_b = vertIndices[b];

			//If vertex pair hasn't already been added
			if (edgeMap.find(VertexPair(a, b)) == edgeMap.end()) {
				edges[j] = mesh->edges.add({
					v_a,	//Head
					{},		//Next
					{},		//Pair
					face	//Face
				});
				edgeMap[VertexPair(b, a)] = mesh->edges[edges[j]].pair = mesh->edges.add({
					v_b,	//Head
					{},		//Next
					edges[j],	//Pair
					{}
				});
				(*mesh)[v_a].edge = edges[i];
			}
			else {
				//mesh->pair((*mesh)[edgeMap[VertexPair(a, b)]]).pair = edgeMap[VertexPair(a, b)];
				(*mesh)[edgeMap[VertexPair(a, b)]].face = face;
				edges[j] = edgeMap[VertexPair(a, b)];
				//edgeMap.erase(VertexPair(a, b));
			}
		}


		(*mesh)[edges[0]].next = edges[1];
		(*mesh)[edges[1]].next = edges[2];
		(*mesh)[edges[2]].next = edges[0];

		(*mesh)[face].edge = edges[0];
	}

	for (auto edgeIter = edgeMap.begin(); edgeIter != edgeMap.end(); edgeIter++) {
		//mesh->edges.remove(edgeIter->second);
		mesh[edgeIter->second].next = mesh.circleVertex(mesh[edgeIter->second].pair)
	}

	return vertIndices.back();
}

template <typename P>
typename SlotMap<Vertex<P>>::Index generateTetrahedron(HalfEdgeMesh<P>& mesh, P a, P b, P c, P d) {
	//Make sure ordering is such that faces point out
	if (dot(cross(b - a, c - a), d - a) > 0) {
		P temp = a;
		a = b;
		b = temp;
	}

//	return mesh->vertices.add({ a, {}});

	auto v_a = mesh.vertices.add({ a, {} });
	auto v_b = mesh.vertices.add({ b, {} });
	auto v_c = mesh.vertices.add({ c, {} });
	auto v_d = mesh.vertices.add({ d, {} });

	auto f_abc = mesh.faces.add({});
	auto e_ab = mesh.edges.add({});
	auto e_bc = mesh.edges.add({});
	auto e_ca = mesh.edges.add({});

	auto f_adb = mesh.faces.add({});
	auto e_ad = mesh.edges.add({});
	auto e_db = mesh.edges.add({});
	auto e_ba = mesh.edges.add({});

	auto f_bdc = mesh.faces.add({});
	auto e_bd = mesh.edges.add({});
	auto e_dc = mesh.edges.add({});
	auto e_cb = mesh.edges.add({});

	auto f_cda = mesh.faces.add({});
	auto e_cd = mesh.edges.add({});
	auto e_da = mesh.edges.add({});
	auto e_ac = mesh.edges.add({});

	mesh[v_a].edge = e_ca;
	mesh[v_b].edge = e_db;
	mesh[v_c].edge = e_dc;
	mesh[v_d].edge = e_cd;
	//Vertices complete

	mesh[f_abc].edge = e_ab;
	mesh[f_adb].edge = e_ad;
	mesh[f_bdc].edge = e_bd;
	mesh[f_cda].edge = e_cd;
	//Faces complete

	mesh[e_ab] = {
		v_b,
		e_bc,
		e_ba,
		f_abc
	};

	mesh[e_bc] = {
		v_c,
		e_ca,
		e_cb,
		f_abc
	};

	mesh[e_ca] = {
		v_a,
		e_ab,
		e_ac,
		f_abc
	};

	mesh[e_ad] = {
		v_d,
		e_db,
		e_da,
		f_adb
	};

	mesh[e_db] = {
		v_b,
		e_ba,
		e_bd,
		f_adb
	};

	mesh[e_ba] = {
		v_a,
		e_ad,
		e_ab,
		f_adb
	};

	mesh[e_bd] = {
		v_d,
		e_dc,
		e_db,
		f_bdc
	};

	mesh[e_dc] = {
		v_c,
		e_cb,
		e_cd,
		f_bdc
	};

	mesh[e_cb] = {
		v_b,
		e_bd,
		e_bc,
		f_bdc
	};

	mesh[e_cd] = {
		v_d,
		e_da,
		e_dc,
		f_cda
	};

	mesh[e_da] = {
		v_a,
		e_ac,
		e_ad,
		f_cda
	};

	mesh[e_ac] = {
		v_c,
		e_cd,
		e_ca,
		f_cda
	};

	return v_a;
}

template<typename P>
void fillBoundary(HalfEdgeMesh<P>& mesh, typename SlotMap<HalfEdge<P>>::Index boundaryEdge, P newPoint) {
	SlotMap<Vertex<P>>::Index newVertex = mesh.vertices.add({ newPoint});
	
	//Temporary until nextOnBoundary is phased out
	//boundaryEdge = mesh[boundaryEdge].pair;

	//SlotMap<HalfEdge<P>>::Index currentEdge = boundaryEdge;
	SlotMap<HalfEdge<P>>::Index lastLeadingEdge;

	std::vector <SlotMap<HalfEdge<P>>::Index> boundaryEdgeList = { boundaryEdge };
	do {
		boundaryEdgeList.push_back(mesh[boundaryEdgeList.back()].next);
	} while (boundaryEdgeList.back() != boundaryEdge);

	boundaryEdgeList.pop_back();

	for (auto currentEdge : boundaryEdgeList) {

		SlotMap<Face<P>>::Index newFace = mesh.faces.add({});

		mesh[currentEdge].face = newFace;

		mesh[currentEdge].next = mesh.edges.add({
			newVertex,	//Head
			{},			//Next
			{},			//Pair
			newFace		//Face
		});

		mesh[mesh[currentEdge].next].next = mesh.edges.add({
			mesh[mesh[currentEdge].pair].head,		//Head
			currentEdge,							//Next
			lastLeadingEdge,						//Pair
			newFace									//Face
		});

		if (lastLeadingEdge)
			mesh[lastLeadingEdge].pair = mesh[mesh[currentEdge].next].next;
		lastLeadingEdge = mesh[currentEdge].next;
		mesh[newFace].edge = currentEdge;
	}
	
	//mesh[mesh[mesh[currentEdge].next].next].pair = lastLeadingEdge;
	mesh[lastLeadingEdge].pair = mesh[mesh[boundaryEdge].next].next;
	mesh[mesh[lastLeadingEdge].pair].pair = lastLeadingEdge;
	mesh[newVertex].edge = mesh[boundaryEdge].next;
}


template<typename P>
void convexHullIteration(HalfEdgeMesh<P>& mesh, const std::vector<P>& points) {
	/*SlotMap<Face<P>>::Index extrudedFace = mesh.faces.random();
	P faceCenter = mesh.center(mesh[extrudedFace]);
	P faceNormal = mesh.normal(mesh[extrudedFace]);

	float maxProjectedDistance = 0.f;
	P furthestPoint;
	for (P point : points) {
		float projectedDistance = dot(faceNormal, faceCenter - point);
		if (projectedDistance > maxProjectedDistance) {
			maxProjectedDistance = projectedDistance;
			furthestPoint = point;
		}
	}

	SlotMap<HalfEdge<P>>::Index boundaryEdge;
	std::vector<SlotMap<Face<P>>::Index> faceDeleteList;
	for (auto face = mesh.faces.begin(); face != mesh.faces.end(); ++face) {
		if (dot(mesh.normal(*face), furthestPoint - mesh.center(*face)) > 0.f)
			faceDeleteList.push_back(face.toIndex());	// boundaryEdge = mesh.deleteFace(face.toIndex());
	}

	for (auto f : faceDeleteList) {
		boundaryEdge = mesh.deleteFace(f);
	}

	if (boundaryEdge) {
		fillBoundary(mesh, boundaryEdge, furthestPoint);
	}*/

	vec3 furthestPoint;

	const float MINIMUM_PROJECTED_DISTANCE = 0.2f;
	float maxProjectedDistance = 0.f;
	const int MIN_FACES_CHECKED = 20;
	const int MAX_FACES_CHECKED = 40;
	int facesChecked = 0;
	do {
		SlotMap<Face<vec3>>::Index extrudedFace = mesh.faces.random();
		vec3 faceCenter = mesh.center(mesh[extrudedFace]);
		vec3 faceNormal = mesh.normal(mesh[extrudedFace]);
		for (vec3 point : points) {
			float projectedDistance = dot(faceNormal, point - faceCenter);
			if (projectedDistance > maxProjectedDistance) {
				maxProjectedDistance = projectedDistance;
				furthestPoint = point;
			}
		}

		facesChecked++;
	} while (
		(maxProjectedDistance <= MINIMUM_PROJECTED_DISTANCE || facesChecked <= MIN_FACES_CHECKED)
		&& facesChecked <= MAX_FACES_CHECKED);

	std::vector<SlotMap<Face<vec3>>::Index> faceDeleteList;
	for (auto face = mesh.faces.begin(); face != mesh.faces.end(); ++face) {
		if (dot(mesh.normal(*face), furthestPoint - mesh.center(*face)) > 0.f) {
			faceDeleteList.push_back(face.toIndex());	
		}
	}
	SlotMap<HalfEdge<P>>::Index boundaryEdge;
	for (auto f : faceDeleteList) {
		boundaryEdge = mesh.deleteFace(f);
	}

	if (boundaryEdge) {
		fillBoundary(mesh, boundaryEdge, furthestPoint);
	}

	for (auto edge = mesh.edges.begin(); edge != mesh.edges.end(); ++edge) {
		if (mesh.isBoundary(edge.toIndex()))
			printf("Holes left in mesh\n");
	}
}