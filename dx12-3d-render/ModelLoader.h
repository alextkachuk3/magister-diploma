#pragma once

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Model.h"
#include "Utils.h"
#include "SceneModel.h"

class ModelLoader
{
public:
    static Model LoadModelFromFile(const std::string& modelPath, const std::string& texturePath);
    static SceneModel LoadSceneModelFromFile(const std::string& modelPath, const std::string& textureDir);
private:
    static V3 ConvertVector(const aiVector3D& vec);
    static V2f ConvertUV(const aiVector3D& vec);
};
