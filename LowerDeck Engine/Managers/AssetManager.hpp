#pragma once
#include <vector>
#include <string>
#include "../Utility/Singleton.hpp"
#include "../Render/Model/Mesh.hpp"

class Model;

class TextureManager;
struct aiScene;
struct aiNode;
struct aiMesh;


// TODO:
class Importer : Singleton<Importer>
{
	friend class Singleton<Importer>;
public:
	Importer() = default;

	void Import(std::string_view Filepath);
	//Model Import(std::string_view Filepath);
	
private:
	void ImportGLTF(std::string_view Filepath);
	void ImportGLB(std::string_view Filepath);
	void ImportFBX(std::string_view Filepath);
	
	void ProcessNode(const aiScene* pScene, const aiNode* pNode, model::Node* ParentNode, DirectX::XMMATRIX ParentMatrix);
	void ProcessMesh();
	void ProcessVertices();
	void ProcessIndices();

	Assimp::Importer m_Importer;

	//std::vector<Vertex> m_Vertices;
	//std::vector<uint32> m_Indices;

};
