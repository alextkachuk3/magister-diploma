#include "Dx12SceneModel.h"

Dx12SceneModel::Dx12SceneModel(SceneModel& sceneModel)
{
	meshes.reserve(sceneModel.meshes.size());
	for (Model& model : sceneModel.meshes)
	{
		meshes.emplace_back(model);
	}
}

Dx12SceneModel::Dx12SceneModel(SceneModel&& sceneModel)
{
	meshes.reserve(sceneModel.meshes.size());
	for (Model& model : sceneModel.meshes)
	{
		meshes.emplace_back(std::move(model));
	}

	sceneModel.Clear();
}

Dx12SceneModel& Dx12SceneModel::operator=(SceneModel& sceneModel)
{
	meshes.clear();
	meshes.reserve(sceneModel.meshes.size());
	for (Model& model : sceneModel.meshes)
	{
		meshes.emplace_back(model);
	}
	return *this;
}

Dx12SceneModel& Dx12SceneModel::operator=(SceneModel&& sceneModel)
{
	meshes.clear();
	meshes.reserve(sceneModel.meshes.size());
	for (Model& model : sceneModel.meshes)
	{
		meshes.emplace_back(std::move(model));
	}

	sceneModel.Clear();
	return *this;
}
