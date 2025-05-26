#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include "Model.h"

class Dx12Model
{
public:
	Dx12Model();
	Dx12Model(const Model& model);
	Dx12Model(Model&& model);

	u32 GetVertexCount() const;
	u32 GetIndexCount() const;

	ID3D12Resource* GetVertexBuffer() const;
	ID3D12Resource* GetIndexBuffer() const;
	const std::vector<ID3D12Resource*>& GetTextures() const;
	const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& GetTextureDescriptors() const;
	const std::vector<Mesh>& GetMeshes() const;

private:
	ID3D12Resource* CreateVertexBuffer(const std::vector<Vertex>& vertices);
	ID3D12Resource* CreateIndexBuffer(const std::vector<u32>& indices);
	ID3D12Resource* CreateTexture(const Texture& texture);

	ID3D12Resource* gpuVertexBuffer = nullptr;
	ID3D12Resource* gpuIndexBuffer = nullptr;
	std::vector<ID3D12Resource*> gpuTextures;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> gpuTextureDescriptors;

	std::vector<Mesh> meshes;
	u32 vertexCount = 0;
	u32 indexCount = 0;
};

