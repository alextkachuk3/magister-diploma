#pragma once

#include <vector>
#include <memory>
#include <Vertex.h>
#include "Texture.h"

class Model
{
public:
	std::vector<Vertex> vertices;
	std::vector<u32> indices;
	std::shared_ptr<Texture> texture;

	static Model CreateCube();
};
