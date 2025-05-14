#include "Dx12GlobalContext.h"
#include <Dx12SceneModel.h>

Dx12GlobalContext::Dx12GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height) : GlobalContext(hInstance, windowTitle, width, height)
{
	IDXGIFactory2* factory = nullptr;
	ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

	ID3D12Debug1* debug;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
	debug->EnableDebugLayer();
	debug->SetEnableGPUBasedValidation(true);

	for (u32 AdapterIndex = 0; factory->EnumAdapters1(AdapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND; ++AdapterIndex)
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

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue, windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain));

	currentFrameIndex = 0;
	ThrowIfFailed(swapChain->GetBuffer(0, IID_PPV_ARGS(&frameBuffers[0])));
	ThrowIfFailed(swapChain->GetBuffer(1, IID_PPV_ARGS(&frameBuffers[1])));

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)));
	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	fenceValue = 0;

	rtvArena = Dx12PlacementHeap(device, D3D12_HEAP_TYPE_DEFAULT, MegaBytes(50), D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);

	uploadArena = Dx12UploadArenaCreate(MegaBytes(500));

	bufferArena = Dx12PlacementHeap(device, D3D12_HEAP_TYPE_DEFAULT, MegaBytes(100), D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);

	textureArena = Dx12PlacementHeap(device, D3D12_HEAP_TYPE_DEFAULT, MegaBytes(1000), D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);

	rtvHeap = Dx12DescriptorHeapCreate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);
	dsvHeap = Dx12DescriptorHeapCreate(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = desc.Format;
	clearValue.DepthStencil.Depth = 1;

	depthBuffer = Dx12CreateResource(&rtvArena, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue);
	depthDescriptor = Dx12DescriptorAllocate(&dsvHeap);
	device->CreateDepthStencilView(depthBuffer, 0, depthDescriptor);

	frameBufferDescriptors[0] = Dx12DescriptorAllocate(&rtvHeap);
	device->CreateRenderTargetView(frameBuffers[0], nullptr, frameBufferDescriptors[0]);
	frameBufferDescriptors[1] = Dx12DescriptorAllocate(&rtvHeap);
	device->CreateRenderTargetView(frameBuffers[1], nullptr, frameBufferDescriptors[1]);
}

void Dx12GlobalContext::Run()
{
	ThrowIfFailed(commandList->Close());
	commandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&commandList);

	WaitForGpu();

	ThrowIfFailed(commandAllocator->Reset());
	ThrowIfFailed(commandList->Reset(commandAllocator, 0));

	Dx12ClearUploadArena(&uploadArena);

	isRunning = true;

	Model fox = ModelLoader::LoadModelFromFile("./assets/fox/Fox.gltf", "./assets/fox/Texture.png");
	SceneModel scene = ModelLoader::LoadSceneModelFromFile("./assets/sponza/Sponza.gltf", "./assets/sponza/textures/");

	Dx12Model dx12fox = std::move(fox);
	Dx12SceneModel dx12Scene = std::move(scene);

	std::vector<Dx12Model> models;

	models.push_back(dx12fox);

	for (const Dx12Model& model : dx12Scene.meshes)
	{
		models.push_back(model);
	}

	while (isRunning)
	{
		TransitionResource(frameBuffers[currentFrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		const FLOAT color[4] = { 1, 0, 1, 1 };
		commandList->ClearRenderTargetView(frameBufferDescriptors[currentFrameIndex], color, 0, nullptr);
		commandList->ClearDepthStencilView(depthDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);

		TransitionResource(frameBuffers[currentFrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		ThrowIfFailed(commandList->Close());
		commandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&commandList);
		swapChain->Present(1, 0);

		fenceValue += 1;
		ThrowIfFailed(commandQueue->Signal(fence, fenceValue));

		WaitForGpu();

		ThrowIfFailed(commandAllocator->Reset());
		ThrowIfFailed(commandList->Reset(commandAllocator, nullptr));

		currentFrameIndex = (currentFrameIndex + 1) % 2;
	}
}

void Dx12GlobalContext::WaitForGpu()
{
	fenceValue += 1;
	ThrowIfFailed(commandQueue->Signal(fence, fenceValue));

	if (fence->GetCompletedValue() != fenceValue)
	{
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}
}

void Dx12GlobalContext::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;

	commandList->ResourceBarrier(1, &barrier);
}

ID3D12Resource* Dx12GlobalContext::Dx12CreateResource(Dx12PlacementHeap* placementHeap, D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE* clearValues)
{
	ID3D12Resource* result = nullptr;

	D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = device->GetResourceAllocationInfo(0, 1, desc);
	u64 gpuAlignedOffset = Utils::Align(placementHeap->used, allocationInfo.Alignment);
	Assert((gpuAlignedOffset + allocationInfo.SizeInBytes) < placementHeap->size);

	ThrowIfFailed(device->CreatePlacedResource(placementHeap->heap, gpuAlignedOffset, desc, initialState, clearValues, IID_PPV_ARGS(&result)));
	placementHeap->used = gpuAlignedOffset + allocationInfo.SizeInBytes;

	return result;
}

ID3D12Resource* Dx12GlobalContext::Dx12CreateBufferAsset(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* bufferData)
{
	return activeInstance ? static_cast<Dx12GlobalContext*>(activeInstance)->Dx12CreateBufferAssetInternal(desc, initialState, bufferData) : nullptr;
}

ID3D12Resource* Dx12GlobalContext::Dx12CreateBufferAssetInternal(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* bufferData)
{
	ID3D12Resource* result = nullptr;

	u64 uploadOffset = 0;
	{
		u8* Dest = Dx12UploadArenaPushSize(&uploadArena, desc->Width, &uploadOffset);
		memcpy(Dest, bufferData, desc->Width);
	}

	result = Dx12CreateResource(&bufferArena, desc, D3D12_RESOURCE_STATE_COPY_DEST, 0);
	commandList->CopyBufferRegion(result, 0, uploadArena.gpuBuffer, uploadOffset, desc->Width);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = result;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = initialState;
	commandList->ResourceBarrier(1, &barrier);

	return result;
}

ID3D12Resource* Dx12GlobalContext::Dx12CreateTextureAsset(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* texels)
{
	return activeInstance ? static_cast<Dx12GlobalContext*>(activeInstance)->Dx12CreateTextureAssetInternal(desc, initialState, texels) : nullptr;
}

ID3D12Resource* Dx12GlobalContext::Dx12CreateTextureAssetInternal(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* texels)
{
	D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = device->GetResourceAllocationInfo(0, 1, desc);
	u64 bytesPerPixel = Utils::Dx12GetBytesPerPixel(desc->Format);
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootPrint = {};

	Assert(desc->DepthOrArraySize == 1);
	D3D12_SUBRESOURCE_FOOTPRINT footPrint = {};
	footPrint.Format = desc->Format;
	footPrint.Width = desc->Width;
	footPrint.Height = desc->Height;
	footPrint.Depth = 1;
	footPrint.RowPitch = Utils::Align(footPrint.Width * bytesPerPixel, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	PlacedFootPrint.Footprint = footPrint;

	u8* DestTexels = Dx12UploadArenaPushSize(&uploadArena, footPrint.Height * footPrint.RowPitch, &PlacedFootPrint.Offset);

	for (u32 Y = 0; Y < desc->Height; ++Y)
	{
		u8* src = (u8*)texels + (Y * desc->Width) * bytesPerPixel;
		u8* dest = DestTexels + (Y * footPrint.RowPitch);
		memcpy(dest, src, bytesPerPixel * desc->Width);
	}

	ID3D12Resource* result = Dx12CreateResource(&textureArena, desc, D3D12_RESOURCE_STATE_COPY_DEST, 0);

	D3D12_TEXTURE_COPY_LOCATION sourceRegion = {};
	sourceRegion.pResource = uploadArena.gpuBuffer;
	sourceRegion.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	sourceRegion.PlacedFootprint = PlacedFootPrint;

	D3D12_TEXTURE_COPY_LOCATION destinationRegion = {};
	destinationRegion.pResource = result;
	destinationRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	destinationRegion.SubresourceIndex = 0;

	commandList->CopyTextureRegion(&destinationRegion, 0, 0, 0, &sourceRegion, nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = result;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = initialState;
	commandList->ResourceBarrier(1, &barrier);

	return result;
}

Dx12DescriptorHeap Dx12GlobalContext::Dx12DescriptorHeapCreate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors)
{
	Dx12DescriptorHeap result = {};

	result.maxElements = numDescriptors;
	result.stepSize = device->GetDescriptorHandleIncrementSize(type);

	D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
	Desc.Type = type;
	Desc.NumDescriptors = numDescriptors;
	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&result.heap)));

	return result;
}

D3D12_CPU_DESCRIPTOR_HANDLE Dx12GlobalContext::Dx12DescriptorAllocate(Dx12DescriptorHeap* descriptorHeap)
{
	Assert(descriptorHeap->currentElement < descriptorHeap->maxElements);
	D3D12_CPU_DESCRIPTOR_HANDLE result = descriptorHeap->heap->GetCPUDescriptorHandleForHeapStart();
	result.ptr += descriptorHeap->stepSize * descriptorHeap->currentElement;

	descriptorHeap->currentElement += 1;

	return result;
}

Dx12UploadBuffer Dx12GlobalContext::Dx12UploadArenaCreate(u64 size)
{
	Dx12UploadBuffer result = {};
	result.size = size;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, 0, IID_PPV_ARGS(&result.gpuBuffer)));
	result.gpuBuffer->Map(0, 0, (void**)&result.cpuPtr);

	return result;
}

u8* Dx12GlobalContext::Dx12UploadArenaPushSize(Dx12UploadBuffer* buffer, u64 size, u64* outOffset)
{
	u64 alignedOffset = Utils::Align(buffer->used, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
	Assert((alignedOffset + size) < buffer->size);

	u8* result = buffer->cpuPtr + alignedOffset;
	buffer->used = alignedOffset + size;
	*outOffset = alignedOffset;

	return result;
}

void Dx12GlobalContext::Dx12ClearUploadArena(Dx12UploadBuffer* buffer)
{
	buffer->used = 0;
}
