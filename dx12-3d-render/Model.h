#pragma once

#include <memory>
#include <vector>
#include "V2.h"
#include "V3.h"
#include "Texture.h"

class Model
{
public:
    std::vector<V3> vertices;
    std::vector<V2f> uvs;
    std::vector<u32> indices;
    std::unique_ptr<Texture> texture;

    Model() = default;
    void LoadCube();
};
