#include "Dx12GlobalContext.h"

Dx12GlobalContext::Dx12GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height) : GlobalContext(hInstance, windowTitle, width, height)
{
	IDXGIFactory2* Factory = 0;
	ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&Factory)));

	ID3D12Debug1* Debug;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
	Debug->EnableDebugLayer();
	Debug->SetEnableGPUBasedValidation(true);

	for (u32 AdapterIndex = 0; Factory->EnumAdapters1(AdapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++AdapterIndex)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		adapter->GetDesc1(&adapterDesc);

		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
		{
			break;
		}
	}

	ID3D12Device* Device = device;

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	ThrowIfFailed(Factory->CreateSwapChainForHwnd(commandQueue, windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain));

	currentFrameIndex = 0;
	ThrowIfFailed(swapChain->GetBuffer(0, IID_PPV_ARGS(&frameBuffers[0])));
	ThrowIfFailed(swapChain->GetBuffer(1, IID_PPV_ARGS(&frameBuffers[1])));

	ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)));
	ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	fenceValue = 0;

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.NumDescriptors = 2;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(Device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&rtvHeap)));

	u32 DescriptorStepSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	frameBufferDescriptors[0] = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	Device->CreateRenderTargetView(frameBuffers[0], nullptr, frameBufferDescriptors[0]);
	frameBufferDescriptors[1].ptr = frameBufferDescriptors[0].ptr + DescriptorStepSize;
	Device->CreateRenderTargetView(frameBuffers[1], nullptr, frameBufferDescriptors[1]);
}

void Dx12GlobalContext::Run()
{
	isRunning = true;
	while (isRunning)
	{
		ProcessSystemMessages();
	}
}
