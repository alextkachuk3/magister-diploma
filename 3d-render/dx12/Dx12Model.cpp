#include "Dx12Model.h"

Dx12Model::Dx12Model()
{
	gpuVertexBuffer = nullptr;
	gpuIndexBuffer = nullptr;
	gpuTexture = nullptr;
	gpuDescriptor.ptr = 0;
}

Dx12Model::Dx12Model(Model& model)
{
	gpuVertexBuffer = CreateVertexBuffer(model.vertices);
	gpuIndexBuffer = CreateIndexBuffer(model.indices);
	gpuTexture = CreateTexture(*model.texture);
	gpuDescriptor = Dx12GlobalContext::TextureDescriptorAllocate(gpuTexture);
}

Dx12Model::Dx12Model(Model&& model)
{
	gpuVertexBuffer = CreateVertexBuffer(model.vertices);
	gpuIndexBuffer = CreateIndexBuffer(model.indices);
	gpuTexture = CreateTexture(*model.texture);

	model.vertices.clear();
	model.indices.clear();
	model.texture.reset();
}

Dx12Model& Dx12Model::operator=(Model& model)
{
	if (this != nullptr) 
	{
		gpuVertexBuffer = CreateVertexBuffer(model.vertices);
		gpuIndexBuffer = CreateIndexBuffer(model.indices);
		gpuTexture = CreateTexture(*model.texture);
	}
	return *this;
}

Dx12Model& Dx12Model::operator=(Model&& model)
{
	if (this != nullptr) 
	{
		gpuVertexBuffer = CreateVertexBuffer(model.vertices);
		gpuIndexBuffer = CreateIndexBuffer(model.indices);
		gpuTexture = CreateTexture(*model.texture);

		model.vertices.clear();
		model.indices.clear();
		model.texture.reset();
	}
	return *this;
}

ID3D12Resource* Dx12Model::CreateVertexBuffer(std::vector<Vertex>& vertices)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = vertices.size() * sizeof(Vertex);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	return Dx12GlobalContext::CreateBufferAsset(&desc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, vertices.data());
}

ID3D12Resource* Dx12Model::CreateIndexBuffer(std::vector<u32>& indices)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = indices.size() * sizeof(u32);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	return Dx12GlobalContext::CreateBufferAsset(&desc, D3D12_RESOURCE_STATE_INDEX_BUFFER, indices.data());
}

ID3D12Resource* Dx12Model::CreateTexture(Texture& texture)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = texture.getWidth();
	desc.Height = texture.getHeight();
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	return Dx12GlobalContext::CreateTextureAsset(&desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, (u8*)texture.getTexels());
}
