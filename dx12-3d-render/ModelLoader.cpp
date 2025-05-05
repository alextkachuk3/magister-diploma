#include "ModelLoader.h"

static V3 ConvertVector(const aiVector3D& vec)
{
    return V3(vec.x, vec.y, vec.z);
}

static V2f ConvertUV(const aiVector3D& vec)
{
    return V2f(vec.x, vec.y);
}

Model ModelLoader::LoadModelFromFile(const std::string& filepath)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        filepath,
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        AssertMsg("Assimp failed to load model");
    }

    Model model;

    aiMesh* mesh = scene->mMeshes[0];

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        model.vertices.push_back(ConvertVector(mesh->mVertices[i]));

        if (mesh->HasTextureCoords(0))
        {
            model.uvs.push_back(ConvertUV(mesh->mTextureCoords[0][i]));
        }
        else
        {
            model.uvs.push_back(V2f(0.0f, 0.0f));
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
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
            std::string directory = filepath.substr(0, filepath.find_last_of("/\\"));
            std::string fullTexPath = directory + "/" + texPath.C_Str();

            model.texture = std::make_unique<Texture>(16, 16);
            model.texture.get()->generateCheckerboardTexture(4);
        }
    }

    return model;
}
