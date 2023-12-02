#include "../../Graphics/TextureManager.hpp"
#include <assimp/scene.h>
#include "Importer.hpp"
#include "../../Utility/FileSystem.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include "../../Utility/Utility.hpp"
#include "../../Graphics/MipMapGenerator.hpp"


Importer::Importer(std::string_view Filepath)
{
	m_ModelPath = Filepath;
}

Importer::~Importer()
{
	for (auto& texture : m_Textures)
		delete texture;

	for (auto& mesh : m_Meshes)
		delete mesh;

	for (auto& material : m_Materials)
		delete material;

}

bool Importer::Import(std::string_view Filepath)
{
	Assimp::Importer importer;
	const auto loadFlags{  
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_JoinIdenticalVertices |
		aiProcess_PreTransformVertices |
		aiProcess_ValidateDataStructure |
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals
	};
	importer.SetExtraVerbose(true);
	const aiScene* scene{ importer.ReadFile(Filepath.data(), loadFlags) };
	
	if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
	{
		utility::ErrorMessage(importer.GetErrorString());
		throw std::runtime_error(importer.GetErrorString());
	}

	ProcessNode(scene, scene->mRootNode, nullptr, XMMatrixIdentity());
	
	if (scene->HasMaterials())
		m_Materials.reserve(scene->mNumMaterials);
	
	if (scene->HasTextures())
		m_Textures.reserve(scene->mNumTextures);
	
	// TODO:
	if (scene->HasAnimations())
	{
		ProcessAnimations(scene);
		bHasAnimations = true;
	}
	
	importer.FreeScene();

	return true;
}

void Importer::ProcessNode(const aiScene* pScene, const aiNode* pNode, model::Node* ParentNode, XMMATRIX ParentMatrix)
{
	model::Node* newNode{ new model::Node() };
	newNode->Parent = ParentNode;
	newNode->Name = std::string(pNode->mName.C_Str());

	const auto transform = [&]() {
		if (!pNode->mTransformation.IsIdentity())
		{
			XMFLOAT4X4 temp{ XMFLOAT4X4() };
			temp._11 = static_cast<float>(pNode->mTransformation.a1);
			temp._12 = static_cast<float>(pNode->mTransformation.a2);
			temp._13 = static_cast<float>(pNode->mTransformation.a3);
			temp._14 = static_cast<float>(pNode->mTransformation.a4);
			temp._21 = static_cast<float>(pNode->mTransformation.b1);
			temp._22 = static_cast<float>(pNode->mTransformation.b2);
			temp._23 = static_cast<float>(pNode->mTransformation.b3);
			temp._24 = static_cast<float>(pNode->mTransformation.b4);
			temp._31 = static_cast<float>(pNode->mTransformation.c1);
			temp._32 = static_cast<float>(pNode->mTransformation.c2);
			temp._33 = static_cast<float>(pNode->mTransformation.c3);
			temp._34 = static_cast<float>(pNode->mTransformation.c4);
			temp._41 = static_cast<float>(pNode->mTransformation.d1);
			temp._42 = static_cast<float>(pNode->mTransformation.d2);
			temp._43 = static_cast<float>(pNode->mTransformation.d3);
			temp._44 = static_cast<float>(pNode->mTransformation.d4);
			newNode->Matrix = XMLoadFloat4x4(&temp);
		}
		else
		{
			aiVector3D		translation;
			aiQuaternion	rotation;
			aiVector3D		scale;

			pNode->mTransformation.Decompose(scale, rotation, translation);
			newNode->Translation = XMFLOAT3(translation.x, translation.y, translation.z);
			newNode->Rotation = XMFLOAT4(rotation.x, rotation.y, rotation.z, rotation.w);
			newNode->Scale = XMFLOAT3(scale.x, scale.y, scale.z);
		}
		};
	transform();
	
	XMMATRIX local{ XMMatrixScalingFromVector(XMLoadFloat3(&newNode->Scale)) * XMMatrixRotationQuaternion(XMLoadFloat4(&newNode->Rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&newNode->Translation))  };
	XMMATRIX next{ local * ParentMatrix };

	if (pNode->mChildren)
	{
		for (size_t i = 0; i < pNode->mNumChildren; i++)
			ProcessNode(pScene, pNode->mChildren[i], newNode, next);
	}

	if (pNode->mMeshes)
	{
		for (uint32_t i = 0; i < pNode->mNumMeshes; i++)
		{
			m_Meshes.emplace_back(ProcessMesh(pScene->mMeshes[pNode->mMeshes[i]], next));
			ProcessMaterials(pScene, pScene->mMeshes[pNode->mMeshes[i]]);
		}
	}
}

model::Mesh* Importer::ProcessMesh(const aiMesh* pMesh, XMMATRIX Matrix)
{
	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT2> uvs;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT3> tangents;
	std::vector<XMFLOAT3> bitangents;

	model::Mesh* newMesh{ new model::Mesh() };

	newMesh->Matrix = Matrix;
	newMesh->Name = std::string(pMesh->mName.data);

	newMesh->BaseVertexLocation  = static_cast<uint32_t>(m_Vertices.size());
	newMesh->FirstIndexLocation  = static_cast<uint32_t>(m_Indices.size());
	newMesh->StartVertexLocation = static_cast<uint32_t>(m_Vertices.size());

	newMesh->VertexCount = static_cast<uint32_t>(pMesh->mNumVertices);

	for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
	{
		if (pMesh->HasPositions())
			positions.emplace_back(pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z);
		else
			positions.emplace_back(0.0f, 0.0f, 0.0f);

		if (pMesh->mTextureCoords[0])
			uvs.emplace_back(pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y);
		else
			uvs.emplace_back(0.0f, 0.0f);
		
		if (pMesh->HasNormals())
			normals.emplace_back(pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z);
		else
			normals.emplace_back(0.0f, 0.0f, 0.0f);

		if (pMesh->HasTangentsAndBitangents())
		{
			tangents.emplace_back(pMesh->mTangents[i].x, pMesh->mTangents[i].y, pMesh->mTangents[i].z);
			bitangents.emplace_back(pMesh->mBitangents[i].x, pMesh->mBitangents[i].y, pMesh->mBitangents[i].z);
		}
		else
		{
			tangents.emplace_back(0.0f, 0.0f, 0.0f);
			bitangents.emplace_back(0.0f, 0.0f, 0.0f);
		}
	}

	newMesh->VertexCount = static_cast<uint32_t>(positions.size());
	for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
		m_Vertices.emplace_back(positions.at(i), uvs.at(i), normals.at(i), tangents.at(i), bitangents.at(i));

	uint32_t indexCount{ 0 };
	if (pMesh->HasFaces())
	{
		newMesh->bHasIndices = true;
		for (uint32_t i = 0; i < pMesh->mNumFaces; i++)
		{
			aiFace& face{ pMesh->mFaces[i] };
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				m_Indices.push_back(face.mIndices[j]);
				indexCount++;
			}
		}
	}

	newMesh->IndexCount = static_cast<uint32_t>(indexCount);

	return newMesh;
}

void Importer::ProcessMaterials(const aiScene* pScene, const aiMesh* pMesh)
{
	model::Material* newMaterial{ new model::Material() };

	if (pMesh->mMaterialIndex < 0)
	{
		m_Materials.emplace_back(newMaterial);
		return;
	}

	auto& textureManager{ TextureManager::GetInstance() };

	aiMaterial* material{ pScene->mMaterials[pMesh->mMaterialIndex] };

	for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); ++i)
	{
		aiString materialPath;
		if (material->GetTexture(aiTextureType_DIFFUSE, i, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath{ utility::glTF::GetTexturePath(m_ModelPath.data(), std::string(materialPath.C_Str())) };

			Texture* baseColorTexture = new Texture(textureManager.Create(texPath));
			m_Textures.emplace_back(baseColorTexture);
			newMaterial->BaseColorIndex = baseColorTexture->SRV().Index;

			aiColor4D colorFactor{};
			aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &colorFactor);
			newMaterial->BaseColorFactor = XMFLOAT4(colorFactor.r, colorFactor.g, colorFactor.b, colorFactor.a);
		}
	}
	
	for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_NORMALS); ++i)
	{
		aiString materialPath;
		if (material->GetTexture(aiTextureType_NORMALS, i, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath{ utility::glTF::GetTexturePath(m_ModelPath.data(), std::string(materialPath.C_Str())) };
			Texture* NormalTexture = new Texture(textureManager.Create(texPath));
			m_Textures.emplace_back(NormalTexture);
			newMaterial->NormalIndex = NormalTexture->SRV().Index;
		}	
	}

	for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_METALNESS); ++i)
	{
		aiString materialPath{};
		if (material->GetTexture(aiTextureType_METALNESS, i, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath{ utility::glTF::GetTexturePath(m_ModelPath.data(), std::string(materialPath.C_Str())) };
			Texture* metallicRoughnessTexture = new Texture(textureManager.Create(texPath));
			m_Textures.emplace_back(metallicRoughnessTexture);
			newMaterial->MetallicRoughnessIndex = metallicRoughnessTexture->SRV().Index;

			aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &newMaterial->MetallicFactor);
			aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &newMaterial->RoughnessFactor);
		}
	}

	for (uint32_t i = 0; i < material->GetTextureCount(aiTextureType_EMISSIVE); ++i)
	{
		aiString materialPath{};
		if (material->GetTexture(aiTextureType_EMISSIVE, i, &materialPath) == aiReturn_SUCCESS)
		{
			auto texPath{ utility::glTF::GetTexturePath(m_ModelPath.data(), std::string(materialPath.C_Str())) };
			Texture* emissiveTexture = new Texture(textureManager.Create(texPath));
			m_Textures.emplace_back(emissiveTexture);
			newMaterial->EmissiveIndex = emissiveTexture->SRV().Index;

			aiColor4D colorFactor{};
			aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &colorFactor);
			newMaterial->EmissiveFactor = XMFLOAT4(colorFactor.r, colorFactor.g, colorFactor.b, colorFactor.a);
		}
	}

	aiGetMaterialFloat(material, AI_MATKEY_GLTF_ALPHACUTOFF, &newMaterial->AlphaCutoff);

	m_Materials.emplace_back(newMaterial);

}

void Importer::ProcessAnimations(const aiScene*)
{
	// TODO: ...
}
