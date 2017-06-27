#define _CRT_SECURE_NO_WARNINGS

#include "ModelLoader.h"

#include "ElementGeometry.h"
#include "TextureMat.h"
#include "ColorMat.h"
#include "ShadedMat.h"
#include "TextureCreation.h"

#include <fstream>
#include <map>
#include <iterator>

//Last three arguments are return values
void sharedIndices2(vector<unsigned int> &_faces, vector<unsigned int> &_nFaces, vector<vec3> &_normals,
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
void sharedIndices2(vector<unsigned int> &_faces, vector<unsigned int> &_nFaces, vector<unsigned int>& _tFaces, vector<vec3>& _normals, vector<vec2>& _uvs,
	//Returned values
	vector<vec3> &vertices, vector<vec3> &normals, vector<vec2>& uvs, vector<unsigned int> &faces)
{
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

			faces.push_back(vertices.size() - 1);
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

struct MeshInfo {
	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec2> texCoords;
	vector<unsigned int> faces;
};

struct MatInfo {
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float ns;
	string texture;
	MatInfo() :ka(), kd(), ks(), ns(), texture() {}
};

struct Vertex {
	vec3 position;
	vec3 normal;
	vec2 texcoord;
	Vertex(vec3 position, vec3 normal, vec2 texcoord) :
		position(position), normal(normal), texcoord(texcoord) {}
};

//Assumes triangulated mesh
bool loadWavefront(std::string directory, std::string filename, std::vector<Drawable> *drawables, TextureManager *manager) {
	FILE *f = fopen((directory+filename+".obj").c_str(), "r");
	if (!f)
	{
		printf("%s does not exist\n", (directory + filename + ".obj").c_str());
		return false;
	}
	else
		printf("Loading %s\n", (directory + filename + ".obj").c_str());

	//String is material name, int is index
	map<string, int> materialMap;	

	vector<MeshInfo> objects;

	vector<vec3> rawVertices;
	vector<vec3> rawNormals;
	vector<vec2> rawTexCoords;
	vector<vector<unsigned int>> rawVFaces = { vector<unsigned int>() };
	vector<vector<unsigned int>> rawNFaces = { vector<unsigned int>() };
	vector<vector<unsigned int>> rawTFaces = { vector<unsigned int>() };

	bool success = true;

	bool noTexCoords = false;
	bool noNormals = false;

	char text[256];
	int fIndex = 0;

	//Debugging
	vector<Vertex> originalVertices;

	while(success && fscanf(f, "%s", text) != EOF){
		//Vertex positions
		if (strcmp(text, "v") == 0) {
			vec3 vert;
			if (fscanf(f, "%f %f %f\n", &vert.x, &vert.y, &vert.z) == 3)
				rawVertices.push_back(vert);
			else {
				printf("Error reading vertex\n");
				success = false;
			}

		}
		//Vertex normals
		else if (strcmp(text, "vn") == 0) {
			vec3 normal;
			if (fscanf(f, "%f %f %f\n", &normal.x, &normal.y, &normal.z) == 3)
				rawNormals.push_back(normal);
			else {
				printf("Error reading normal\n");
				success = false;
			}
		}
		//Vertex texture coordinates
		else if (strcmp(text, "vt") == 0){
			vec2 tex;
			if (fscanf(f, "%f %f\n", &tex.x, &tex.y) == 2)
				rawTexCoords.push_back(tex);
			else {
				printf("Error reading texture coordinate\n");
				success = false;
			}
		}
		//Faces
		else if (strcmp(text, "f") == 0) {
			unsigned int v1, v2, v3, u1, u2, u3, n1, n2, n3;
			if (rawTexCoords.size() != 0 && rawNormals.size() != 0) {
				if (fscanf(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
					&v1, &u1, &n1, &v2, &u2, &n2, &v3, &u3, &n3) != 9) {
					printf("Error reading face\n");
					success = false;
				}
			}
			else if (rawTexCoords.size() != 0) {
				if (fscanf(f, "%d//%d %d//%d %d//%d",
					&v1, &n1, &v2, &n2, &v3, &n3) != 9) {
					printf("Error reading face\n");
					success = false;
				}
			}

			rawVFaces[fIndex].push_back(v1 - 1);
			rawVFaces[fIndex].push_back(v2 - 1);
			rawVFaces[fIndex].push_back(v3 - 1);

			if (rawTexCoords.size() != 0) {
				rawTFaces[fIndex].push_back(u1 - 1);
				rawTFaces[fIndex].push_back(u2 - 1);
				rawTFaces[fIndex].push_back(u3 - 1);
			}
			if (rawNormals.size() != 0) {
				rawNFaces[fIndex].push_back(n1 - 1);
				rawNFaces[fIndex].push_back(n2 - 1);
				rawNFaces[fIndex].push_back(n3 - 1);
			}

			originalVertices.push_back(
				Vertex(rawVertices[v1-1], rawNormals[n1-1], rawTexCoords[u1-1]));
			originalVertices.push_back(
				Vertex(rawVertices[v2-1], rawNormals[n2-1], rawTexCoords[u2-1]));
			originalVertices.push_back(
				Vertex(rawVertices[v3-1], rawNormals[n3-1], rawTexCoords[u3-1]));
		}
		//Use material - @TODO Modify to split up objects made of same material?
		else if (strcmp(text, "usemtl") == 0) {
			char temp[250];
			fscanf(f, "%s", temp);
			if (materialMap.find(string(temp)) == materialMap.end()) {
				materialMap[string(temp)] = materialMap.size();
				fIndex = materialMap.size() - 1;
				if(materialMap.size() > rawVFaces.size()){
					rawVFaces.push_back({});
					rawNFaces.push_back({});
					rawTFaces.push_back({});
				}
			}
			else 
				fIndex = materialMap[string(temp)];
		}
	}

	fclose(f);

	//Read material file
	f = fopen((directory+filename + ".mtl").c_str(), "r");

	if (f == nullptr) {
		printf("No Material file found\n");
	}
	else
		printf("Loading %s\n", (directory + filename + ".mtl").c_str());

	vector<MatInfo> materials(rawVFaces.size());

	if(f) {
		char temp[256];
		int currentMtl = 0;

		//Parse material file
		while (fscanf(f, "%s", text) != EOF) {
			if (strcmp(text, "newmtl") == 0) {
				char mtlName[256];
				fscanf(f, "%s", mtlName);
				try {
					currentMtl = materialMap.at(mtlName);
				} catch (out_of_range) {
					currentMtl = -1;
					printf("Material not found - %s\n", mtlName);
				}
			}
			if (strcmp(text, "Ka") == 0 && currentMtl >= 0){
				vec3 &ka = materials[currentMtl].ka;
				if (fscanf(f, "%f %f %f\n", &ka.x, &ka.y, &ka.z) != 3) {
					printf("Error reading Ka\n");
				}
			}
			if (strcmp(text, "Kd") == 0 && currentMtl >= 0) {
				vec3 &kd = materials[currentMtl].kd;
				if (fscanf(f, "%f %f %f", &kd.x, &kd.y, &kd.z) != 3) {
					printf("Error reading Kd\n");
				}
			}
			if (strcmp(text, "Ks") == 0 && currentMtl >= 0) {
				vec3 &ks = materials[currentMtl].ks;
				if (fscanf(f, "%f %f %f", &ks.x, &ks.y, &ks.z) != 3) {
					printf("Error reading Ks\n");
				}
			}
			if (strcmp(text, "Ns") == 0 && currentMtl >= 0) {
				if (fscanf(f, "%f", &materials[currentMtl].ns) != 1) {
					printf("Error reading Ns\n");
				}
			}
			if (strcmp(text, "map_Kd") == 0 && currentMtl >= 0) {
				char temp[256];
				if (fscanf(f, "%s", temp) == 1)
					materials[currentMtl].texture = temp;
				else
					printf("Error reading map_Kd\n");
			}
		}
	}

	fclose(f);

	map<string, Texture> textures;

	//Debugging
	vector<Vertex> newVertices;

	//Split into drawables
	for (auto it = materialMap.begin(); it != materialMap.end(); it++) {
		int mat = it->second;
		vector<vec3> verts;
		vector<vec2> texCoords;
		vector<vec3> normals;
		vector<unsigned int> vFaces;
		vector<unsigned int> tFaces;
		vector<unsigned int> nFaces;

		map<int, int> fMap;
		//Vertices
		for (int i = 0; i < rawVFaces[mat].size(); i++) {
			if (fMap.find(rawVFaces[mat][i]) == fMap.end()) {
				verts.push_back(rawVertices[rawVFaces[mat][i]]);
				fMap[rawVFaces[mat][i]] = verts.size() - 1;
				vFaces.push_back(verts.size() - 1);
			}
			else
				vFaces.push_back(fMap[rawVFaces[mat][i]]);
		}
		fMap.clear();

		//Normals
		for (int i = 0; i < rawNFaces[mat].size(); i++) {
			if (fMap.find(rawNFaces[mat][i]) == fMap.end()) {
				normals.push_back(rawNormals[rawNFaces[mat][i]]);
				fMap[rawNFaces[mat][i]] = normals.size() - 1;
				nFaces.push_back(normals.size() - 1);
			}
			else
				nFaces.push_back(fMap[rawNFaces[mat][i]]);
		}
		fMap.clear();

		//Texture coords
		for (int i = 0; i < rawTFaces[mat].size(); i++) {
			if (fMap.find(rawTFaces[mat][i]) == fMap.end()) {
				texCoords.push_back(rawTexCoords[rawTFaces[mat][i]]);
				fMap[rawTFaces[mat][i]] = texCoords.size() - 1;
				tFaces.push_back(texCoords.size() - 1);
			}
			else
				tFaces.push_back(fMap[rawTFaces[mat][i]]);
		}

		MeshInfo object;
		object.vertices = verts;
		
		if (vFaces.size() != 0 && tFaces.size() != 0) {
			sharedIndices2(vFaces, nFaces, tFaces, normals, texCoords,
				object.vertices, object.normals, object.texCoords, object.faces);
		}
		//Add other cases

		float ka = length(materials[mat].ka);
		float kd = length(materials[mat].kd);
		float ks = length(materials[mat].ks);
		float sum = max(ka + kd + ks, 1.f);
		ka /= sum;
		kd /= sum;
		ks /= sum;

		drawables->push_back(Drawable(
			new ShadedMat(
				ka, kd, ks,
				max(materials[mat].ns*50.f, 1.f)),
			new ElementGeometry(
				&object.vertices[0],
				&object.normals[0],
				&object.texCoords[0],
				&object.faces[0],
				object.vertices.size(),
				object.faces.size())
		));

		if (materials[mat].texture.length() > 0) {
			auto texIt = textures.find(materials[mat].texture);
			Texture tex;
			if (texIt == textures.end()) {
				tex = createTexture2D((directory + materials[mat].texture).c_str(), manager);
				textures[materials[mat].texture] = tex;
			}
			else
				tex = texIt->second;

			drawables->back().addMaterial(new TextureMat(tex));
		}
		else {
			drawables->back().addMaterial(new ColorMat(normalize(materials[mat].kd)));
		}

		//Debugging
		for (int i = 0; i < object.faces.size(); i++) {
			newVertices.push_back(Vertex(
				object.vertices[object.faces[i]],
				object.normals[object.faces[i]],
				object.texCoords[object.faces[i]]));
		}
	}

	/*for (int i = 0; i < newVertices.size(); i++) {
		if (newVertices[i].position != originalVertices[i].position)
			printf("Position error (%f %f %f) -> (%f %f %f)\n",
				originalVertices[i].position.x, originalVertices[i].position.y, originalVertices[i].position.z,
				newVertices[i].position.x, newVertices[i].position.y, newVertices[i].position.z);
		if(newVertices[i].normal != newVertices[i].normal)
			printf("Normal error (%f %f %f) -> (%f %f %f)\n",
				originalVertices[i].normal.x, originalVertices[i].normal.y, originalVertices[i].normal.z,
				newVertices[i].normal.x, newVertices[i].normal.y, newVertices[i].normal.z);

		if (newVertices[i].texcoord != newVertices[i].texcoord)
			printf("texcoord error (%f %f) -> (%f %f)\n",
				originalVertices[i].texcoord.x, originalVertices[i].texcoord.y,
				newVertices[i].texcoord.x, newVertices[i].texcoord.y);
	}*/

	return true;
}
