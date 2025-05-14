#pragma once

#include "d3d12.h"
#include <dxgi1_6.h>
#include "GlobalContext.h"
#include "Dx12PlacementHeap.h"
#include <Dx12UploadBuffer.h>
#include <Dx12Model.h>
#include <Dx12DescriptorHeap.h>

class Dx12GlobalContext : public GlobalContext
{
public:
	Dx12GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height);

	void Run() override;

	static ID3D12Resource* Dx12CreateBufferAsset(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* bufferData);
	static ID3D12Resource* Dx12CreateTextureAsset(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* texels);

private:
	IDXGIAdapter1* adapter;
	ID3D12Device* device;
	ID3D12CommandQueue* commandQueue;

	Dx12UploadBuffer uploadArena;
	Dx12PlacementHeap rtvArena;
	Dx12PlacementHeap bufferArena;
	Dx12PlacementHeap textureArena;

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

	void WaitForGpu();
	void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	ID3D12Resource* Dx12CreateResource(Dx12PlacementHeap* placementHeap, D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE* clearValues);
	ID3D12Resource* Dx12CreateBufferAssetInternal(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* bufferData);
	ID3D12Resource* Dx12CreateTextureAssetInternal(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* texels);
	Dx12DescriptorHeap Dx12DescriptorHeapCreate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors);
	D3D12_CPU_DESCRIPTOR_HANDLE Dx12DescriptorAllocate(Dx12DescriptorHeap* descriptorHeap);
	Dx12UploadBuffer Dx12UploadArenaCreate(u64 size);
	u8* Dx12UploadArenaPushSize(Dx12UploadBuffer* buffer, u64 size, u64* outOffset);
	void Dx12ClearUploadArena(Dx12UploadBuffer* buffer);
};
