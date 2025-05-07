#pragma once

#include <vector>
#include <memory>
#include "V2.h"
#include "V3.h"
#include "Texture.h"

class Model
{
public:
	std::vector<V3> vertices;
	std::vector<V2f> uvs;
	std::vector<u32> indices;
	std::shared_ptr<Texture> texture;

	static Model CreateCube();
};
