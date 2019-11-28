#include <VertexAttribLayout.h>
#include <cstring>

namespace renderlib{
/*
std::string VertexPosition::name() { return "VertexPosition"; }
std::string VertexPosition2D::name() { return "VertexPosition2D"; }
std::string VertexNormal::name() { return "VertexNormal"; }
std::string VertexTexCoord::name() { return "VertexTexCoord"; }
*/
VertexBinding::VertexBinding() {}
VertexBinding::VertexBinding(const std::vector<int>& attribLocations) :attribLocations(attribLocations) {}

bool VertexBinding::operator==(const VertexBinding& other) const {
	return attribLocations == other.attribLocations;
}

bool VertexBinding::operator!=(const VertexBinding& other) const {
	return !(*this == other);
}

};