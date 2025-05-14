#include "ModelLoader.h"

V3 ModelLoader::ConvertVector(const aiVector3D& vec)
{
    return V3(vec.x, vec.y, vec.z);
}

V2f ModelLoader::ConvertUV(const aiVector3D& vec)
{
    return V2f(vec.x, vec.y);
}

Model ModelLoader::LoadModelFromFile(const std::string& modelPath, const std::string& texturePath)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		modelPath,
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		AssertMsg("Failed to load model file");
	}

	Model model;
	aiMesh* mesh = scene->mMeshes[0];

	for (u32 i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		vertex.position = ConvertVector(mesh->mVertices[i]);

		if (mesh->HasTextureCoords(0))
			vertex.uv = ConvertUV(mesh->mTextureCoords[0][i]);
		else
			vertex.uv = V2f(0.0f, 0.0f);

		model.vertices.push_back(vertex);
	}

	for (u32 i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		for (u32 j = 0; j < face.mNumIndices; ++j)
		{
			model.indices.push_back(face.mIndices[j]);
		}
	}

	model.texture = std::make_shared<Texture>(Texture::LoadFromFile(texturePath));

	return model;
}

SceneModel ModelLoader::LoadSceneModelFromFile(const std::string& modelPath, const std::string& textureDir)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		modelPath,
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices);

	if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		AssertMsg("Failed to load scene");
	}

	SceneModel sceneModel;

	for (u32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		Model model;

		model.indices.reserve(mesh->mNumFaces * 3);
		model.vertices.reserve(mesh->mNumVertices);

		for (u32 i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex vertex;
			vertex.position = ConvertVector(mesh->mVertices[i]);
			if (mesh->HasTextureCoords(0))
				vertex.uv = ConvertUV(mesh->mTextureCoords[0][i]);
			else
				vertex.uv = V2f(0.0f, 0.0f);
			model.vertices.push_back(vertex);
		}

		for (u32 i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];
			for (u32 j = 0; j < face.mNumIndices; ++j)
			{
				model.indices.push_back(face.mIndices[j]);
			}
		}

		if (scene->HasMaterials() && mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiString texPath;

			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
			{
				std::string fullTexPath = textureDir + "/" + std::string(texPath.C_Str());
				model.texture = std::make_shared<Texture>(Texture::LoadFromFile(fullTexPath));
			}
			else
			{
				model.texture = std::make_shared<Texture>(Texture::generateCheckerboardTexture(64, 64, 8, Colors::Purple, Colors::Black));
			}
		}

		sceneModel.meshes.push_back(std::move(model));
	}

	return sceneModel;
}
