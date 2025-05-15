#pragma once

#include "d3d12.h"
#include <dxgi1_6.h>
#include "GlobalContext.h"
#include "Dx12PlacementHeap.h"
#include <Dx12UploadBuffer.h>
#include <Dx12Model.h>
#include <Dx12DescriptorHeap.h>
#include "Dx12ShaderBytecode.h"

class Dx12GlobalContext : public GlobalContext
{
public:
	Dx12GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height);

	

	void Run() override;

	static ID3D12Resource* CreateBufferAsset(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* bufferData);
	static ID3D12Resource* CreateTextureAsset(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* texels);
	static D3D12_GPU_DESCRIPTOR_HANDLE TextureDescriptorAllocate(ID3D12Resource* gpuTexture);

private:
	IDXGIAdapter1* adapter;
	ID3D12Device* device;
	ID3D12CommandQueue* commandQueue;

	Dx12UploadBuffer uploadBuffer;
	Dx12PlacementHeap rtvPlacement;
	Dx12PlacementHeap bufferPlacement;
	Dx12PlacementHeap texturePlacement;

	IDXGISwapChain1* swapChain;
	u32 currentFrameIndex;
	ID3D12Resource* frameBuffers[2];
	D3D12_CPU_DESCRIPTOR_HANDLE frameBufferDescriptors[2];

	D3D12_CPU_DESCRIPTOR_HANDLE depthDescriptor;
	ID3D12Resource* depthBuffer;

	ID3D12CommandAllocator* commandAllocator;

	ID3D12GraphicsCommandList* commandList;
	ID3D12Fence* fence;
	UINT64 fenceValue;

	Dx12DescriptorHeap rtvHeap;
	Dx12DescriptorHeap dsvHeap;	
	Dx12DescriptorHeap shaderDescriptorHeap;

	ID3D12Resource* transformBuffer;
	D3D12_GPU_DESCRIPTOR_HANDLE transformDescriptor;

	ID3D12RootSignature* modelRootSignature;
	ID3D12PipelineState* modelPipeline;

	void CreateRootSignatureAndPipelineState(Dx12ShaderBytecode& vertexShader, Dx12ShaderBytecode& pixelShader);
	void CreateTransformBuffer();

	void WaitForGpu();
	void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	ID3D12Resource* CreateResource(Dx12PlacementHeap* placementHeap, D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE* clearValues);
	ID3D12Resource* CreateBufferAssetInternal(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* bufferData);
	ID3D12Resource* CreateTextureAssetInternal(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* texels);
	Dx12DescriptorHeap DescriptorHeapCreate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	void DescriptorAllocate(Dx12DescriptorHeap* heap, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle);
	void CopyDataToBuffer(D3D12_RESOURCE_STATES startState, D3D12_RESOURCE_STATES endState, void* data, u64 dataSize, ID3D12Resource* gpuBuffer);
	Dx12UploadBuffer UploadBufferCreate(u64 size);
	u8* UploadArenaPushSize(Dx12UploadBuffer* buffer, u64 size, u64* outOffset);
	void ClearUploadBuffer(Dx12UploadBuffer* buffer);
};
