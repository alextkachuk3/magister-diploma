#include "Dx12Model.h"
#include "Dx12GlobalContext.h"

Dx12Model::Dx12Model(const Model& model)
{
	vertexCount = static_cast<u32>(model.vertices.size());
	indexCount = static_cast<u32>(model.indices.size());
	meshes = model.meshes;

	gpuVertexBuffer = CreateVertexBuffer(model.vertices);
	gpuIndexBuffer = CreateIndexBuffer(model.indices);

	for (const Texture& texture : model.textures)
	{
		ID3D12Resource* textureResourse = CreateTexture(texture);
		gpuTextures.push_back(textureResourse);
		gpuTextureDescriptors.push_back(Dx12GlobalContext::TextureDescriptorAllocate(textureResourse));
	}
}

Dx12Model::Dx12Model(Model&& model)
{
	vertexCount = static_cast<u32>(model.vertices.size());
	indexCount = static_cast<u32>(model.indices.size());
	meshes = std::move(model.meshes);

	gpuVertexBuffer = CreateVertexBuffer(model.vertices);
	gpuIndexBuffer = CreateIndexBuffer(model.indices);

	for (Texture& texture : model.textures)
	{
		ID3D12Resource* textureResourse = CreateTexture(texture);
		gpuTextures.push_back(textureResourse);
		gpuTextureDescriptors.push_back(Dx12GlobalContext::TextureDescriptorAllocate(textureResourse));
	}
	model.vertices.clear();
	model.indices.clear();
	model.textures.clear();
	model.meshes.clear();
}

u32 Dx12Model::GetVertexCount() const { return vertexCount; }
u32 Dx12Model::GetIndexCount() const { return indexCount; }
ID3D12Resource* Dx12Model::GetVertexBuffer() const { return gpuVertexBuffer; }
ID3D12Resource* Dx12Model::GetIndexBuffer() const { return gpuIndexBuffer; }
const std::vector<ID3D12Resource*>& Dx12Model::GetTextures() const { return gpuTextures; }
const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>& Dx12Model::GetTextureDescriptors() const { return gpuTextureDescriptors; }
const std::vector<Mesh>& Dx12Model::GetMeshes() const { return meshes; }

ID3D12Resource* Dx12Model::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = static_cast<UINT64>(vertices.size() * sizeof(Vertex));
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	return Dx12GlobalContext::CreateBufferAsset(&desc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, vertices.data());
}

ID3D12Resource* Dx12Model::CreateIndexBuffer(const std::vector<u32>& indices)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = static_cast<UINT64>(indices.size() * sizeof(u32));
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	return Dx12GlobalContext::CreateBufferAsset(&desc, D3D12_RESOURCE_STATE_INDEX_BUFFER, static_cast<const void*>(indices.data()));
}

ID3D12Resource* Dx12Model::CreateTexture(const Texture& texture)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = texture.getWidth();
	desc.Height = texture.getHeight();
	desc.DepthOrArraySize = 1;
	desc.MipLevels = static_cast<UINT16>(ceil(log2(max(desc.Width, desc.Height)))) + 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	return Dx12GlobalContext::CreateTextureAsset(&desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, (u8*)texture.getTexels());
}
