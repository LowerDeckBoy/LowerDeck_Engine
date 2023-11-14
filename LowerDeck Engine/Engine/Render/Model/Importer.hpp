#pragma once
#include <vector>
#include <string>
#include "Mesh.hpp"

using namespace DirectX;

class TextureManager;
struct aiScene;
struct aiNode;
struct aiMesh;

class Importer
{
public:
	Importer() = default;
	Importer(std::string_view Filepath);
	virtual ~Importer();

	bool Import(std::string_view Filepath);

	void ProcessNode(const aiScene* pScene, const aiNode* pNode, model::Node* ParentNode, XMMATRIX ParentMatrix);
	model::Mesh* ProcessMesh(const aiMesh* pMesh, XMMATRIX Matrix);
	void ProcessMaterials(const aiScene* pScene, const aiMesh* pMesh);

	void ProcessAnimations(const aiScene* pScene);

protected:
	std::string_view m_ModelPath;

	std::string m_ModelName;

	std::vector<gfx::Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	//std::vector<model::Node*> m_Nodes;
	std::vector<model::Mesh*> m_Meshes;

	std::vector<model::Material*> m_Materials;
	std::vector<Texture*> m_Textures;
	//std::vector<model::MaterialData*> m_Materials;

	bool bHasAnimations{ false };

};
