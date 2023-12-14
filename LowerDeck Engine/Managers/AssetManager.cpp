#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>

#include "AssetManager.hpp"

#include "../Utility/FileSystem.hpp"

void Importer::Import(std::string_view Filepath)
{
	if (utility::GetFileExtension(Filepath) == ".gltf")
	{
		ImportGLTF(Filepath);
	}
	else if (utility::GetFileExtension(Filepath) == ".glb")
	{
		ImportGLB(Filepath);
	}
	else if (utility::GetFileExtension(Filepath) == ".fbx")
	{
		ImportFBX(Filepath);
	}


}

void Importer::ImportGLTF(std::string_view Filepath)
{
	
	const auto loadFlags{
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_JoinIdenticalVertices |
		aiProcess_PreTransformVertices |
		aiProcess_ValidateDataStructure |
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals
	};
	m_Importer.SetExtraVerbose(true);



	m_Importer.FreeScene();

}

