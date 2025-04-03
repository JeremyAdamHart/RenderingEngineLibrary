#pragma once
#include <memory>



template<typename Data, size_t N>
class NTree {
public:
	std::unique_ptr<NTree<Data, N>> children[N];
	NTree<Data, N>* parent;

	Data v;
	NTree<Data, N>& child(size_t q) { return children[q]; }
};

/*
* ---------
* | 0 | 1 |
* ---------
* | 3 | 2 |
* ---------
*/

enum Quadrant : size_t {
	TL = 0,
	TR,
	BR,
	BL
};

template<typename Data>
using Quadtree = NTree<Data, 4>;