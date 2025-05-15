#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include "Model.h"
#include <Dx12GlobalContext.h>

class Dx12Model
{
public:
	Dx12Model();
	Dx12Model(Model& model);
	Dx12Model(Model&& model);

	Dx12Model& operator=(Model& model);
	Dx12Model& operator=(Model&& model);

	ID3D12Resource* gpuVertexBuffer;
	ID3D12Resource* gpuIndexBuffer;
	ID3D12Resource* gpuTexture;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor;

	u32 GetVertexCount() const;
	u32 GetIndexCount() const;

private:
	ID3D12Resource* CreateVertexBuffer(std::vector<Vertex>& vertices);
	ID3D12Resource* CreateIndexBuffer(std::vector<u32>& indices);
	ID3D12Resource* CreateTexture(Texture& texture);

	u32 vertexCount = 0;
	u32 indexCount = 0;
};
