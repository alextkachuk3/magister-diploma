#include "ModelLoader.h"

V3 ModelLoader::ConvertVector(const aiVector3D& vec)
{
    return V3(vec.x, vec.y, vec.z);
}

V2f ModelLoader::ConvertUV(const aiVector3D& vec)
{
    return V2f(vec.x, vec.y);
}

Model ModelLoader::LoadModelFromFile(const std::string& modelPath, const std::string& textureDir)
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

    Model model;

    size_t estimatedVertexCount = 0;
    size_t estimatedIndexCount = 0;
    for (u32 i = 0; i < scene->mNumMeshes; ++i)
    {
        estimatedVertexCount += scene->mMeshes[i]->mNumVertices;
        estimatedIndexCount += scene->mMeshes[i]->mNumFaces * 3;
    }
    model.vertices.reserve(estimatedVertexCount);
    model.indices.reserve(estimatedIndexCount);
    model.meshes.reserve(scene->mNumMeshes);
    model.meshTextureIds.reserve(scene->mNumMeshes);

    std::unordered_map<std::string, u32> textureCache;

    for (u32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh* aiMeshPtr = scene->mMeshes[meshIndex];

        Mesh outMesh{};
        outMesh.vertexOffset = static_cast<u32>(model.vertices.size());
        outMesh.indexOffset = static_cast<u32>(model.indices.size());
        outMesh.vertexCount = aiMeshPtr->mNumVertices;

        for (u32 i = 0; i < aiMeshPtr->mNumVertices; ++i)
        {
            Vertex vertex;
            vertex.position = ConvertVector(aiMeshPtr->mVertices[i]);

            if (aiMeshPtr->HasTextureCoords(0))
                vertex.uv = ConvertUV(aiMeshPtr->mTextureCoords[0][i]);
            else
                vertex.uv = V2f(0.0f, 0.0f);

            if (aiMeshPtr->HasNormals())
                vertex.normal = ConvertVector(aiMeshPtr->mNormals[i]);
            else
                vertex.normal = V3(0.0f, 1.0f, 0.0f);

            model.vertices.push_back(vertex);
        }

        for (u32 i = 0; i < aiMeshPtr->mNumFaces; ++i)
        {
            const aiFace& face = aiMeshPtr->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; ++j)
            {
                model.indices.push_back(face.mIndices[j]);
            }
        }

        outMesh.indexCount = static_cast<u32>(model.indices.size()) - outMesh.indexOffset;

        u32 textureId = 0;
        if (scene->HasMaterials() && aiMeshPtr->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[aiMeshPtr->mMaterialIndex];
            aiString texPath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
            {
                std::string fullTexPath = textureDir + "/" + std::string(texPath.C_Str());

                auto it = textureCache.find(fullTexPath);
                if (it != textureCache.end())
                {
                    textureId = it->second;
                }
                else
                {
                    model.textures.push_back(Texture::LoadFromFile(fullTexPath));
                    textureId = static_cast<u32>(model.textures.size() - 1);
                    textureCache[fullTexPath] = textureId;
                }
            }
            else
            {
                std::string fallbackKey = "__checkerboard__";
                auto it = textureCache.find(fallbackKey);
                if (it != textureCache.end())
                {
                    textureId = it->second;
                }
                else
                {
                    model.textures.push_back(Texture::generateCheckerboardTexture(64, 64, 8, Colors::Purple, Colors::Black));
                    textureId = static_cast<u32>(model.textures.size() - 1);
                    textureCache[fallbackKey] = textureId;
                }
            }
        }

        outMesh.textureId = textureId;
        model.meshes.push_back(outMesh);
        model.meshTextureIds.push_back({ textureId });
    }

    return model;
}
