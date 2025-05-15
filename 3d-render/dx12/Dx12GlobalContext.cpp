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

	rtvPlacement = Dx12PlacementHeap(device, D3D12_HEAP_TYPE_DEFAULT, MegaBytes(500), D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);

	uploadBuffer = UploadBufferCreate(MegaBytes(500));

	bufferPlacement = Dx12PlacementHeap(device, D3D12_HEAP_TYPE_DEFAULT, MegaBytes(500), D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);

	texturePlacement = Dx12PlacementHeap(device, D3D12_HEAP_TYPE_DEFAULT, MegaBytes(500), D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);

	rtvHeap = DescriptorHeapCreate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);
	dsvHeap = DescriptorHeapCreate(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
	shaderDescriptorHeap = DescriptorHeapCreate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 150, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

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

	depthBuffer = CreateResource(&rtvPlacement, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue);
	DescriptorAllocate(&dsvHeap, &depthDescriptor, nullptr);
	device->CreateDepthStencilView(depthBuffer, 0, depthDescriptor);

	DescriptorAllocate(&rtvHeap, &frameBufferDescriptors[0], nullptr);
	device->CreateRenderTargetView(frameBuffers[0], nullptr, frameBufferDescriptors[0]);
	DescriptorAllocate(&rtvHeap, &frameBufferDescriptors[1], nullptr);
	device->CreateRenderTargetView(frameBuffers[1], nullptr, frameBufferDescriptors[1]);

	Dx12ShaderBytecode vertexShader(L"./VsMain.cso");
	Dx12ShaderBytecode pixelShader(L"./PsMain.cso");

	CreateRootSignatureAndPipelineState(vertexShader, pixelShader);
	CreateTransformBuffer();
}

void Dx12GlobalContext::CreateRootSignatureAndPipelineState(Dx12ShaderBytecode& vertexShader, Dx12ShaderBytecode& pixelShader)
{
	{
		D3D12_ROOT_PARAMETER RootParameters[2] = {};

		D3D12_DESCRIPTOR_RANGE Table1Range[1] = {};
		{
			Table1Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			Table1Range[0].NumDescriptors = 1;
			Table1Range[0].BaseShaderRegister = 0;
			Table1Range[0].RegisterSpace = 0;
			Table1Range[0].OffsetInDescriptorsFromTableStart = 0;

			RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			RootParameters[0].DescriptorTable.NumDescriptorRanges = ArraySize(Table1Range);
			RootParameters[0].DescriptorTable.pDescriptorRanges = Table1Range;
			RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}

		D3D12_DESCRIPTOR_RANGE Table2Range[1] = {};
		{
			Table2Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			Table2Range[0].NumDescriptors = 1;
			Table2Range[0].BaseShaderRegister = 0;
			Table2Range[0].RegisterSpace = 0;
			Table2Range[0].OffsetInDescriptorsFromTableStart = 0;

			RootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			RootParameters[1].DescriptorTable.NumDescriptorRanges = ArraySize(Table2Range);
			RootParameters[1].DescriptorTable.pDescriptorRanges = Table2Range;
			RootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}

		D3D12_STATIC_SAMPLER_DESC StaticSamplerDesc = {};
		StaticSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		StaticSamplerDesc.ShaderRegister = 0;
		StaticSamplerDesc.RegisterSpace = 0;
		StaticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC SignatureDesc = {};
		SignatureDesc.NumParameters = ArraySize(RootParameters);
		SignatureDesc.pParameters = RootParameters;
		SignatureDesc.NumStaticSamplers = 1;
		SignatureDesc.pStaticSamplers = &StaticSamplerDesc;
		SignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3DBlob* SerializedRootSig = 0;
		ID3DBlob* ErrorBlob = 0;
		ThrowIfFailed(D3D12SerializeRootSignature(&SignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1_0,
			&SerializedRootSig,
			&ErrorBlob));
		ThrowIfFailed(device->CreateRootSignature(0,
			SerializedRootSig->GetBufferPointer(),
			SerializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&modelRootSignature)));

		if (SerializedRootSig)
		{
			SerializedRootSig->Release();
		}
		if (ErrorBlob)
		{
			ErrorBlob->Release();
		}
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.pRootSignature = modelRootSignature;

	desc.VS = vertexShader.GetBytecode();
	desc.PS = pixelShader.GetBytecode();

	desc.BlendState.RenderTarget[0].BlendEnable = true;
	desc.BlendState.RenderTarget[0].LogicOpEnable = false;
	desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	desc.SampleMask = 0xFFFFFFFF;
	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	desc.RasterizerState.FrontCounterClockwise = FALSE;
	desc.RasterizerState.DepthClipEnable = TRUE;

	desc.DepthStencilState.DepthEnable = TRUE;
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[0].InputSlot = 0;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].InputSlot = 0;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

	desc.InputLayout.pInputElementDescs = inputElementDescs;
	desc.InputLayout.NumElements = ArraySize(inputElementDescs);

	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&modelPipeline)));
}

void Dx12GlobalContext::CreateTransformBuffer()
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = Utils::Align(sizeof(M4), 256);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	transformBuffer = CreateResource(&bufferPlacement, &desc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = transformBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = static_cast<UINT>(desc.Width);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {};
	DescriptorAllocate(&shaderDescriptorHeap, &cpuDescriptor, &transformDescriptor);
	device->CreateConstantBufferView(&cbvDesc, cpuDescriptor);
}

void Dx12GlobalContext::Run()
{
	isRunning = true;

	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency(&timerFrequency);

	LARGE_INTEGER beginTime;
	QueryPerformanceCounter(&beginTime);

	Model fox = ModelLoader::LoadModelFromFile("./assets/fox/Fox.gltf", "./assets/fox/Texture.png");
	SceneModel scene = ModelLoader::LoadSceneModelFromFile("./assets/sponza/Sponza.gltf", "./assets/sponza/textures/");

	Dx12Model dx12fox = std::move(fox);
	Dx12SceneModel dx12Scene = std::move(scene);

	std::vector<Dx12Model> models;

	for (const Dx12Model& model : dx12Scene.meshes)
	{
		models.push_back(model);
	}

	ThrowIfFailed(commandList->Close());
	commandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&commandList);

	WaitForGpu();

	ThrowIfFailed(commandAllocator->Reset());
	ThrowIfFailed(commandList->Reset(commandAllocator, 0));

	ClearUploadBuffer(&uploadBuffer);

	while (isRunning)
	{
		LARGE_INTEGER endTime;
		QueryPerformanceCounter(&endTime);
		f32 frameTime = static_cast<f32>(endTime.QuadPart - beginTime.QuadPart) / static_cast<f32>(timerFrequency.QuadPart);
		beginTime = endTime;

		frameTimeLogger.LogFrameTime(frameTime);

		TransitionResource(frameBuffers[currentFrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		const FLOAT color[4] = { 1, 0, 1, 1 };
		commandList->ClearRenderTargetView(frameBufferDescriptors[currentFrameIndex], color, 0, nullptr);
		commandList->ClearDepthStencilView(depthDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);

		M4 transform = M4::Perspective(aspectRatio, 1.57f, 0.01f, 4000.0f) * camera.getCameraTransformMatrix() * M4::Translation(0, 0, 10) * M4::Rotation(0, 0, 0) * M4::Scale(1.0f, 1.0f, 1.0f);

		CopyDataToBuffer(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &transform, sizeof(M4), transformBuffer);

		commandList->OMSetRenderTargets(1, frameBufferDescriptors + currentFrameIndex, 0, &depthDescriptor);

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = frameBufferWidthF32;
		viewport.Height = frameBufferHeightF32;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		commandList->RSSetViewports(1, &viewport);

		D3D12_RECT ScissorRect = {};
		ScissorRect.left = 0;
		ScissorRect.right = frameBufferWidth;
		ScissorRect.top = 0;
		ScissorRect.bottom = frameBufferHeight;
		commandList->RSSetScissorRects(1, &ScissorRect);

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetGraphicsRootSignature(modelRootSignature);
		commandList->SetPipelineState(modelPipeline);
		commandList->SetDescriptorHeaps(1, &shaderDescriptorHeap.heap);
		commandList->SetGraphicsRootDescriptorTable(1, transformDescriptor);

		for (const Dx12Model& model : models)
		{
			D3D12_INDEX_BUFFER_VIEW indexView = {};
			indexView.BufferLocation = model.gpuIndexBuffer->GetGPUVirtualAddress();
			indexView.SizeInBytes = sizeof(u32) * model.GetIndexCount();
			indexView.Format = DXGI_FORMAT_R32_UINT;
			commandList->IASetIndexBuffer(&indexView);

			D3D12_VERTEX_BUFFER_VIEW vertexView = {};
			vertexView.BufferLocation = model.gpuVertexBuffer->GetGPUVirtualAddress();
			vertexView.SizeInBytes = sizeof(Vertex) * model.GetVertexCount();
			vertexView.StrideInBytes = sizeof(Vertex);
			commandList->IASetVertexBuffers(0, 1, &vertexView);

			commandList->SetGraphicsRootDescriptorTable(0, model.gpuDescriptor);

			commandList->DrawIndexedInstanced(model.GetIndexCount(), 1, 0, 0, 0);
		}

		TransitionResource(frameBuffers[currentFrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		ThrowIfFailed(commandList->Close());
		commandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&commandList);
		swapChain->Present(1, 0);

		WaitForGpu();

		ThrowIfFailed(commandAllocator->Reset());
		ThrowIfFailed(commandList->Reset(commandAllocator, nullptr));

		currentFrameIndex = (currentFrameIndex + 1) % 2;
		ClearUploadBuffer(&uploadBuffer);

		camera.UpdateMouseControl(windowHandle);
		camera.UpdateViewMatrix(frameTime, wButtonPressed, aButtonPressed, sButtonPressed, dButtonPressed);

		ProcessSystemMessages();
	}
}

void Dx12GlobalContext::WaitForGpu()
{
	fenceValue += 1;
	ThrowIfFailed(commandQueue->Signal(fence, fenceValue));

	if (fence->GetCompletedValue() != fenceValue)
	{
		HANDLE fenceEvent = {};
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

ID3D12Resource* Dx12GlobalContext::CreateResource(Dx12PlacementHeap* placementHeap, D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE* clearValues)
{
	ID3D12Resource* result = nullptr;

	D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = device->GetResourceAllocationInfo(0, 1, desc);
	u64 gpuAlignedOffset = Utils::Align(placementHeap->used, allocationInfo.Alignment);
	Assert((gpuAlignedOffset + allocationInfo.SizeInBytes) < placementHeap->size);

	ThrowIfFailed(device->CreatePlacedResource(placementHeap->heap, gpuAlignedOffset, desc, initialState, clearValues, IID_PPV_ARGS(&result)));
	placementHeap->used = gpuAlignedOffset + allocationInfo.SizeInBytes;

	return result;
}

ID3D12Resource* Dx12GlobalContext::CreateBufferAsset(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* bufferData)
{
	return activeInstance ? static_cast<Dx12GlobalContext*>(activeInstance)->CreateBufferAssetInternal(desc, initialState, bufferData) : nullptr;
}

ID3D12Resource* Dx12GlobalContext::CreateBufferAssetInternal(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* bufferData)
{
	u64 uploadOffset = 0;
	{
		u8* Dest = UploadArenaPushSize(&uploadBuffer, desc->Width, &uploadOffset);
		memcpy(Dest, bufferData, desc->Width);
	}

	ID3D12Resource* result = CreateResource(&bufferPlacement, desc, D3D12_RESOURCE_STATE_COPY_DEST, 0);
	commandList->CopyBufferRegion(result, 0, uploadBuffer.gpuBuffer, uploadOffset, desc->Width);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = result;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = initialState;
	commandList->ResourceBarrier(1, &barrier);

	return result;
}

ID3D12Resource* Dx12GlobalContext::CreateTextureAsset(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* texels)
{
	return activeInstance ? static_cast<Dx12GlobalContext*>(activeInstance)->CreateTextureAssetInternal(desc, initialState, texels) : nullptr;
}

ID3D12Resource* Dx12GlobalContext::CreateTextureAssetInternal(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, void* texels)
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

	u8* DestTexels = UploadArenaPushSize(&uploadBuffer, footPrint.Height * footPrint.RowPitch, &PlacedFootPrint.Offset);

	for (u32 Y = 0; Y < desc->Height; ++Y)
	{
		u8* src = (u8*)texels + (Y * desc->Width) * bytesPerPixel;
		u8* dest = DestTexels + (Y * footPrint.RowPitch);
		memcpy(dest, src, bytesPerPixel * desc->Width);
	}

	ID3D12Resource* result = CreateResource(&texturePlacement, desc, D3D12_RESOURCE_STATE_COPY_DEST, 0);

	D3D12_TEXTURE_COPY_LOCATION sourceRegion = {};
	sourceRegion.pResource = uploadBuffer.gpuBuffer;
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

Dx12DescriptorHeap Dx12GlobalContext::DescriptorHeapCreate(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	Dx12DescriptorHeap result = {};

	result.maxElements = numDescriptors;
	result.stepSize = device->GetDescriptorHandleIncrementSize(type);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = type;
	desc.NumDescriptors = numDescriptors;
	desc.Flags = flags;
	ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&result.heap)));

	return result;
}

D3D12_GPU_DESCRIPTOR_HANDLE Dx12GlobalContext::TextureDescriptorAllocate(ID3D12Resource* gpuTexture)
{
	if (activeInstance == nullptr)
	{
		return {};
	}

	Dx12GlobalContext* context = static_cast<Dx12GlobalContext*>(activeInstance);

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 1, 2, 3);
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.PlaneSlice = 0;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor = {};
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {};
	context->DescriptorAllocate(&context->shaderDescriptorHeap, &cpuDescriptor, &gpuDescriptor);
	context->device->CreateShaderResourceView(gpuTexture, &desc, cpuDescriptor);
	return gpuDescriptor;
}

void Dx12GlobalContext::DescriptorAllocate(Dx12DescriptorHeap* heap, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
{
	Assert(heap->currentElement < heap->maxElements);

	if (outCpuHandle)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = heap->heap->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += heap->stepSize * heap->currentElement;
		*outCpuHandle = cpuHandle;
	}

	if (outGpuHandle)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = heap->heap->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += heap->stepSize * heap->currentElement;
		*outGpuHandle = gpuHandle;
	}

	heap->currentElement += 1;
}

void Dx12GlobalContext::CopyDataToBuffer(D3D12_RESOURCE_STATES startState, D3D12_RESOURCE_STATES endState, void* data, u64 dataSize, ID3D12Resource* gpuBuffer)
{
	u64 uploadOffset = 0;

	u8* dest = UploadArenaPushSize(&uploadBuffer, dataSize, &uploadOffset);
	memcpy(dest, data, dataSize);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = gpuBuffer;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = startState;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	commandList->ResourceBarrier(1, &barrier);

	commandList->CopyBufferRegion(gpuBuffer, 0, uploadBuffer.gpuBuffer, uploadOffset, dataSize);

	barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = gpuBuffer;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = endState;
	commandList->ResourceBarrier(1, &barrier);

}

Dx12UploadBuffer Dx12GlobalContext::UploadBufferCreate(u64 size)
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

u8* Dx12GlobalContext::UploadArenaPushSize(Dx12UploadBuffer* buffer, u64 size, u64* outOffset)
{
	u64 alignedOffset = Utils::Align(buffer->used, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
	Assert((alignedOffset + size) < buffer->size);

	u8* result = buffer->cpuPtr + alignedOffset;
	buffer->used = alignedOffset + size;
	*outOffset = alignedOffset;

	return result;
}

void Dx12GlobalContext::ClearUploadBuffer(Dx12UploadBuffer* buffer)
{
	buffer->used = 0;
}
