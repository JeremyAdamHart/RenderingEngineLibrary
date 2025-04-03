#include "VertexVertex.h"

namespace renderlib {
namespace vv { 

void VertexAdjacencyBuilder::addVertices(unsigned int v0, unsigned int v1) {
	int insertedAtFront = -1;
	int insertedAtBack = -1;
	int index = 0;
	for (auto& chain : neighbourChains) {
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
		neighbourChains.push_back({ v0, v1 });
		return;
	}
	//If pair conected two chains
	else if (insertedAtFront >= 0 && insertedAtBack >= 0) {
		auto& frontChain = neighbourChains[insertedAtBack];
		auto& backChain = neighbourChains[insertedAtFront];
		frontChain.pop_back();		//Remove duplicated element at back
		frontChain.insert(frontChain.end(), backChain.begin()+1, backChain.end());
		neighbourChains.erase(neighbourChains.begin() + insertedAtFront);
	}
}

}
}