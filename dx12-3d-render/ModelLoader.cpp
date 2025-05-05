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
		model.vertices.push_back(ConvertVector(mesh->mVertices[i]));

		if (mesh->HasTextureCoords(0))
			model.uvs.push_back(ConvertUV(mesh->mTextureCoords[0][i]));
		else
			model.uvs.emplace_back(0.0f, 0.0f);
	}

	for (u32 i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		for (u32 j = 0; j < face.mNumIndices; ++j)
		{
			model.indices.push_back(face.mIndices[j]);
		}
	}

	model.texture = Texture::LoadFromFile(texturePath);

	return model;
}
