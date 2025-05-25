#pragma once

#include <vector>
#include <memory>
#include "Texture.h"
#include "Vertex.h"
#include "Mesh.h"

class Model
{
public:
    std::vector<Vertex> vertices;

    std::vector<u32> indices;

    std::vector<Texture> textures;

    std::vector<Mesh> meshes;

    std::vector<std::vector<u32>> meshTextureIds;

    void Clear();

    static Model CreateCube();
};
