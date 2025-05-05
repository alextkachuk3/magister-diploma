#pragma once
#include <vector>
#include "Model.h"

class SceneModel
{
public:
	std::vector<Model> meshes;

	void Clear();
};
