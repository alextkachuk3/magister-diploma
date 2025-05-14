#pragma once
#include <vector>
#include <Dx12Model.h>
#include "Dx12SceneModel.h"

class Dx12SceneModel
{
public:
	Dx12SceneModel() = default;
	Dx12SceneModel(SceneModel& sceneModel);
	Dx12SceneModel(SceneModel&& sceneModel);

	Dx12SceneModel& operator=(SceneModel& sceneModel);
	Dx12SceneModel& operator=(SceneModel&& sceneModel);

	std::vector<Dx12Model> meshes;
};
