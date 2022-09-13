// File originally partially created by Troy Alderson for use in CPSC453 assignments. Reused with permission from Ben Stephenson

/*
* Authors - Mia MacTavish and Jeremy Hart
*/

#include "MeshInfoLoader.h"
#include <../tinyply/source/tinyply.h>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <iostream>

#pragma warning(disable:4996)

namespace renderlib {

using namespace std;
using namespace glm;

//Last three arguments are return values
void sharedIndices(vector<unsigned int> &_faces, vector<unsigned int> &_nFaces, vector<vec3> &_normals,
	vector<vec3> &vertices, vector<vec3> &normals, vector<unsigned int> &faces)
{

	normals.resize(vertices.size(), vec3(0.0, 0.0, 0.0));

	faces.clear();

	for (unsigned int i = 0; i<_faces.size(); i++)
	{
		int vi = _faces[i];
		int ni = _nFaces[i];

		if (normals[vi] == vec3())
		{
			normals[vi] = _normals[ni];
			faces.push_back(vi);
		}
		else if (normals[vi] != _normals[ni])
		{
			vertices.push_back(vertices[vi]);
			normals.push_back(_normals[ni]);
			faces.push_back(vertices.size() - 1);
		}
		else
			faces.push_back(vi);
	}
}

//Vertices not modified
void sharedIndices(vector<unsigned int> &_faces, vector<unsigned int> &_nFaces, vector<unsigned int>& _tFaces, vector<vec3>& _normals, vector<vec2>& _uvs, 
	//Returned values
	vector<vec3> &vertices, vector<vec3> &normals, vector<vec2>& uvs, vector<unsigned int> &faces)
{
	vector<vec3> oldVertices = vertices;

	faces.clear();
	uvs.clear();
	normals.clear();

	normals.resize(vertices.size(), vec3(0.0, 0.0, 0.0));

	vector<unsigned int> tempFaces;
	vector<unsigned int> tempTFaces;

	for (unsigned int i = 0; i<_faces.size(); i++)
	{
		int vi = _faces[i];
		int ni = _nFaces[i];
		int ti = _tFaces[i];

		if (normals[vi] == vec3())
		{
			normals[vi] = _normals[ni];
			tempFaces.push_back(vi);
			tempTFaces.push_back(ti);
		}
		else if (normals[vi] != _normals[ni])
		{
			vertices.push_back(vertices[vi]);
			normals.push_back(_normals[ni]);
			
			tempFaces.push_back(vertices.size() - 1);
			tempTFaces.push_back(ti);
		}
		else
		{
			tempFaces.push_back(vi);
			tempTFaces.push_back(ti);
		}
	}

	uvs.resize(vertices.size(), vec2(-1.f, -1.f));

	for (unsigned int i = 0; i < tempFaces.size(); i++)
	{
		int vi = tempFaces[i];
		int ti = tempTFaces[i];

		if (uvs[vi] == vec2(-1.f, -1.f))
		{
			uvs[vi] = _uvs[ti];
			faces.push_back(vi);
		}
		else if (uvs[vi] != _uvs[ti])
		{
			vertices.push_back(vertices[vi]);
			normals.push_back(normals[vi]);
			uvs.push_back(_uvs[ti]);

			faces.push_back(vertices.size() -1);
		}
		else
			faces.push_back(vi);
	}

	//Check validity of mesh
	bool valid = true;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		int unifiedIndex = faces[i];
		int vIndex = _faces[i];
		int nIndex = _nFaces[i];
		int tIndex = _tFaces[i];


		if (vertices[unifiedIndex] != vertices[vIndex])
			valid = false;
		if (normals[unifiedIndex] != _normals[nIndex])
			valid = false;
		if (uvs[unifiedIndex] != _uvs[tIndex])
			valid = false;	
	}

	if (valid == false)
		printf("Mesh continuity violated\n");
}

MeshInfoLoader::MeshInfoLoader(const char *filename) {
	loadModel(filename);
}

vector<vec3> calculateNormalsImp(vector<vec3>* points, vector<unsigned int>* indices) {
	vector<vec3> normals;
	normals.resize(points->size(), vec3(0));
	for (int i = 0; i < indices->size(); i += 3) {
		unsigned int ai = indices->at(i);
		unsigned int bi = indices->at(i + 1);
		unsigned int ci = indices->at(i + 2);
		vec3 a = points->at(ai);
		vec3 b = points->at(bi);
		vec3 c = points->at(ci);

		vec3 normal = cross(b - a, c - a);
		normals[ai] += normal;
		normals[bi] += normal;
		normals[ci] += normal;
	}

	for (int i = 0; i < normals.size(); i++) {
		normals[i] = normalize(normals[i]);
	}

	return normals;
}

bool MeshInfoLoader::loadModelPly(const char *filename) {
	std::ifstream ss (filename, std::ios::binary);

	using namespace tinyply;

	PlyFile file;
	file.parse_header(ss);

	std::shared_ptr<PlyData> positionData, normalData, texcoordData, colorData, faceData;

	try {
		positionData = file.request_properties_from_element("vertex", { "x", "y", "z" }); 
	}
	catch (const std::exception e) { std::cout << "No vertices in mesh - " << e.what() << std::endl; }

	try { 
		normalData = file.request_properties_from_element("vertex", { "nx", "ny", "nz" });
	}catch (const std::exception e) { std::cout << "No normals in mesh - " << e.what() << std::endl; }

	try {
		texcoordData = file.request_properties_from_element("vertex", { "u", "v" });
	}
	catch (const std::exception e) {
		try {
			texcoordData = file.request_properties_from_element("vertex", { "s", "t" });
		}
		catch (const std::exception e) {
			std::cout << "No texture coordinates in mesh - " << e.what() << std::endl;
		}
	}
	try {
		colorData = file.request_properties_from_element("vertex", { "red", "green", "blue" });
	}
	catch (const std::exception e) { std::cout << "No color coordinates in mesh - " << e.what() << std::endl; }
	try {
		faceData = file.request_properties_from_element("face", { "vertex_indices" });
	}
	catch (const std::exception e) { std::cout << "No faces in mesh - " << e.what() << std::endl; }


	file.read(ss);

	if (positionData) {
		vertices.resize(positionData->count);
		std::memcpy(vertices.data(), positionData->buffer.get(), positionData->buffer.size_bytes());
	}
	if (faceData) {
		indices.resize(faceData->count*3);
		size_t byteSize = faceData->buffer.size_bytes();
		std::memcpy(indices.data(), faceData->buffer.get(), faceData->buffer.size_bytes());
	}
	if (normalData) {
		normals.resize(normalData->count);
		std::memcpy(normals.data(), normalData->buffer.get(), normalData->buffer.size_bytes());
	}
	else {
		normals = calculateNormalsImp(&vertices, &indices);
	}
	if (texcoordData) {
		uvs.resize(texcoordData->count);
		std::memcpy(uvs.data(), texcoordData->buffer.get(), texcoordData->buffer.size_bytes());
	}
	if (colorData) {
		std::vector<unsigned char> colorChars;
		colorChars.resize(colorData->count*3);
		std::memcpy(colorChars.data(), colorData->buffer.get(), colorData->buffer.size_bytes());
		for (int i = 0; i + 2 < colorChars.size(); i+=3) {
			colors.push_back(vec3(float(colorChars[i]), float(colorChars[i + 1]), float(colorChars[i + 2])) / 255.f);
		}
	}

	return positionData && faceData;
}

bool MeshInfoLoader::loadModel(const char *filename)
{
	FILE* f = fopen(filename, "r");
	if (f == NULL)
	{
		printf("File %s could not be opened\n", filename);
		return false;
	}

	//vector<GLv3f> vertices;
	vector<vec3> _normals;
	vector<vec2> _uvs;
	vector<unsigned int> _faces;
	vector<unsigned int> _uvfaces;
	vector<unsigned int> _nfaces;

	bool noUVs = false;
	bool noNormals = false;

	while (true)
	{
		char text[256];

		int res = fscanf(f, "%s", text);
		if (res == EOF)
			break;

		if (strcmp(text, "v") == 0)
		{
			vec3 vert;

			fscanf(f, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
			vertices.push_back(vert);
		}
		else if (strcmp(text, "vn") == 0)
		{
			vec3 norm;

			fscanf(f, "%f %f %f\n", &norm.x, &norm.y, &norm.z);
			_normals.push_back(norm);
		}
		else if (strcmp(text, "vt") == 0)
		{
			vec2 uv;

			fscanf(f, "%f %f\n", &uv.x, &uv.y);
			_uvs.push_back(uv);
		}
		else if (strcmp(text, "f") == 0)
		{
			unsigned int vi1, vi2, vi3, ui1, ui2, ui3, ni1, ni2, ni3;

			if ((_uvs.size() == 0) || fscanf(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
				&vi1, &ui1, &ni1, &vi2, &ui2, &ni2, &vi3, &ui3, &ni3) != 9)
			{
				if ((_normals.size() == 0) || fscanf(f, "%d//%d %d//%d %d//%d",
					&vi1, &ni1, &vi2, &ni2, &vi3, &ni3) != 6) {
					if (fscanf(f, "%d %d %d", &vi1, &vi2, &vi3) != 3) {
						printf("Face couldn't be read\n");
						return false;
					}

					noNormals = true;
					noUVs = true;
				}

				noUVs = true;
			}

			_faces.push_back(vi1 - 1);
			_faces.push_back(vi2 - 1);
			_faces.push_back(vi3 - 1);

			if (!noUVs) {
				_uvfaces.push_back(ui1 - 1);
				_uvfaces.push_back(ui2 - 1);
				_uvfaces.push_back(ui3 - 1);
			}

			if (!noNormals) {
				_nfaces.push_back(ni1 - 1);
				_nfaces.push_back(ni2 - 1);
				_nfaces.push_back(ni3 - 1);
			}
		}
	}

	if (noUVs && !noNormals){
		sharedIndices(_faces, _nfaces, _normals, vertices, normals, indices);
		uvs.resize(vertices.size(), vec2(0, 0));
	}
	else if(!noNormals)
		sharedIndices(_faces, _nfaces, _uvfaces, _normals, _uvs, vertices, normals, uvs, indices);
	if (noUVs && noNormals) {
		indices = _faces;
		normals = calculateNormalsImp(&vertices, &indices);
		//normals.resize(vertices.size(), vec3(0, 0, 0));
		uvs.resize(vertices.size(), vec2(0, 0));
	}

	fclose(f);
	return true;
}

void MeshInfoLoader::clearData() {
	vertices.clear();
	normals.clear();
	uvs.clear();
	indices.clear();
}

vec3 MeshInfoLoader::getCenter(){
	vec3 center;

	for (unsigned int i = 0; i<vertices.size(); i++)
	{
		center = center + vertices[i];
	}

	float divisor = (1.f / (vertices.size()));

	return center * divisor;
}

float MeshInfoLoader::getBottom(){
	float bottom = 1000.0;

	for (unsigned int i = 0; i<vertices.size(); i++)
	{
		bottom = std::min(bottom, vertices[i].z);
	}

	return bottom;
}

float MeshInfoLoader::getBoundingRadius()
{
	vec3 center = getCenter();
	float boundingRadius = 0.0;

	for (unsigned int i = 0; i<vertices.size(); i++)
	{
		vec3 point = vertices[i];
		float distance = length(center - point);
		boundingRadius = std::max(boundingRadius, distance);
	}

	return boundingRadius;
}

sptr<MeshGeometryType> objToElementGeometry(char *filename) {
	MeshInfoLoader minfo(filename);
	
	return make<MeshGeometryType>(GL_TRIANGLES, minfo.indices.data(), minfo.indices.size(), 
		minfo.vertices.data(), minfo.normals.data(), minfo.uvs.data(), minfo.vertices.size());
}

void writeToPly(const char* filename, std::vector<glm::vec3>* vertices, std::vector<unsigned int>* faces) {
	std::filebuf fb;
	fb.open(filename, std::ios::out | std::ios::binary);
	std::ostream outstream(&fb);
	if (outstream.fail()) throw std::runtime_error("failed to open " + string(filename));

	using namespace tinyply;

	PlyFile plyOutput;

	plyOutput.add_properties_to_element("vertex", { "x", "y", "z" },
		Type::FLOAT32, vertices->size(), reinterpret_cast<uint8_t*>(vertices->data()), Type::INVALID, 0);
	plyOutput.add_properties_to_element("face", { "vertex_indices" },
		Type::UINT32, faces->size() / 3, reinterpret_cast<uint8_t*>(faces->data()), Type::UINT8, 3);

	plyOutput.write(outstream, true);

	fb.close();
}



}