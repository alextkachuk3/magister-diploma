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

    Model sceneModel;

    for (u32 meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh* aiMeshPtr = scene->mMeshes[meshIndex];

        Mesh outMesh{};
        outMesh.VertexOffset = static_cast<u32>(sceneModel.vertices.size());
        outMesh.IndexOffset = static_cast<u32>(sceneModel.indices.size());
        outMesh.VertexCount = aiMeshPtr->mNumVertices;

        for (u32 i = 0; i < aiMeshPtr->mNumVertices; ++i)
        {
            Vertex vertex;
            vertex.position = ConvertVector(aiMeshPtr->mVertices[i]);

            if (aiMeshPtr->HasTextureCoords(0))
                vertex.uv = ConvertUV(aiMeshPtr->mTextureCoords[0][i]);
            else
                vertex.uv = V2f(0.0f, 0.0f);

            sceneModel.vertices.push_back(vertex);
        }

        for (u32 i = 0; i < aiMeshPtr->mNumFaces; ++i)
        {
            const aiFace& face = aiMeshPtr->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; ++j)
            {
                sceneModel.indices.push_back(face.mIndices[j]);
            }
        }

        outMesh.IndexCount = static_cast<u32>(sceneModel.indices.size()) - outMesh.IndexOffset;

        u32 textureId = 0;
        if (scene->HasMaterials() && aiMeshPtr->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[aiMeshPtr->mMaterialIndex];
            aiString texPath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
            {
                std::string fullTexPath = textureDir + "/" + std::string(texPath.C_Str());

                bool found = false;
                for (u32 i = 0; i < sceneModel.textures.size(); ++i)
                {
                    if (sceneModel.textures[i].getWidth() > 0 &&
                        fullTexPath == texPath.C_Str())
                    {
                        textureId = i;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    sceneModel.textures.push_back(Texture::LoadFromFile(fullTexPath));
                    textureId = static_cast<u32>(sceneModel.textures.size() - 1);
                }
            }
            else
            {
                sceneModel.textures.push_back(Texture::generateCheckerboardTexture(64, 64, 8, Colors::Purple, Colors::Black));
                textureId = static_cast<u32>(sceneModel.textures.size() - 1);
            }
        }

        outMesh.TextureId = textureId;
        sceneModel.meshes.push_back(outMesh);
        sceneModel.meshTextureIds.push_back({ textureId });
    }

    return sceneModel;
}

