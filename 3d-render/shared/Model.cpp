#include "Model.h"

void Model::Clear()
{
    vertices.clear();
    indices.clear();
    textures.clear();
    meshes.clear();
    meshTextureIds.clear();
}

Model Model::CreateCube()
{
	Model model;

	model.vertices =
	{
		// Front
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}}, {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}}, {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}}, {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		// Back
		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}}, {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}}, {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}}, {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
		// Left
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}}, {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}}, {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}}, {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		// Right
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}}, {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}}, {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}}, {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
		// Top
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}}, {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}}, {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}}, {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		// Bottom
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}}, {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}}, {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}}, {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}}
	};

	model.indices =
	{
		 0,  1,  2,  2,  3,  0, // Front
		 4,  5,  6,  6,  7,  4, // Back
		 8,  9, 10, 10, 11,  8, // Left
		12, 13, 14, 14, 15, 12, // Right
		16, 17, 18, 18, 19, 16, // Top
		20, 21, 22, 22, 23, 20  // Bottom
	};

	model.textures.push_back(Texture::generateCheckerboardTexture(16, 16, 2, Colors::Black, Colors::Purple));

	Mesh cubeMesh;
	cubeMesh.vertexOffset = 0;
	cubeMesh.vertexCount = static_cast<u32>(model.vertices.size());
	cubeMesh.indexOffset = 0;
	cubeMesh.indexCount = static_cast<u32>(model.indices.size());
	cubeMesh.textureId = 0;

	model.meshes.push_back(cubeMesh);
	model.meshTextureIds.push_back({ 0 });

	return model;
}
