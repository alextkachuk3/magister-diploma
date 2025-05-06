#pragma once

#include "GlobalContext.h"
#include <wrl.h>
#include "d3d12.h"
#include <dxgi1_6.h>

class Dx12GlobalContext : public GlobalContext
{
public:
	Dx12GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height);

	void Run() override;

private:
    IDXGIAdapter1* adapter;
    ID3D12Device* device;
    ID3D12CommandQueue* commandQueue;

    IDXGISwapChain1* swapChain;
    u32 currentFrameIndex;
    ID3D12Resource* frameBuffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE frameBufferDescriptors[2];

    ID3D12CommandAllocator* commandAllocator;

    ID3D12GraphicsCommandList* commandList;
    ID3D12Fence* fence;
    UINT64 fenceValue;

    ID3D12DescriptorHeap* rtvHeap;
};
