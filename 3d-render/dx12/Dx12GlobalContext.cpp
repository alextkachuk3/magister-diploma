#include "Dx12GlobalContext.h"

Dx12GlobalContext::Dx12GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height) : GlobalContext(hInstance, windowTitle, width, height)
{
	IDXGIFactory2* factory = nullptr;
	ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

	//ID3D12Debug1* debug;
	//ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
	//debug->EnableDebugLayer();
	//debug->SetEnableGPUBasedValidation(true);

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
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

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

	frameBufferWidth = width;
	frameBufferHeight = height;
	frameBufferWidthF32 = static_cast<f32>(frameBufferWidth);
	frameBufferHeightF32 = static_cast<f32>(frameBufferHeight);
	aspectRatio = frameBufferWidthF32 / frameBufferHeightF32;

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
	CreateLightBuffer();
}

void Dx12GlobalContext::CreateRootSignatureAndPipelineState(Dx12ShaderBytecode& vertexShader, Dx12ShaderBytecode& pixelShader)
{
	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	D3D12_DESCRIPTOR_RANGE table1Range[1] = {};

	table1Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	table1Range[0].NumDescriptors = 1;
	table1Range[0].BaseShaderRegister = 0;
	table1Range[0].RegisterSpace = 0;
	table1Range[0].OffsetInDescriptorsFromTableStart = 0;

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = ArraySize(table1Range);
	rootParameters[0].DescriptorTable.pDescriptorRanges = table1Range;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_DESCRIPTOR_RANGE table2Range[1] = {};

	table2Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	table2Range[0].NumDescriptors = 2;
	table2Range[0].BaseShaderRegister = 0;
	table2Range[0].RegisterSpace = 0;
	table2Range[0].OffsetInDescriptorsFromTableStart = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = ArraySize(table2Range);
	rootParameters[1].DescriptorTable.pDescriptorRanges = table2Range;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_STATIC_SAMPLER_DESC staticSamplerDesc = {};
	staticSamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	staticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplerDesc.MaxAnisotropy = 16.0f;
	staticSamplerDesc.MinLOD = 0;
	staticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplerDesc.ShaderRegister = 0;
	staticSamplerDesc.RegisterSpace = 0;
	staticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC signatureDesc = {};
	signatureDesc.NumParameters = ArraySize(rootParameters);
	signatureDesc.pParameters = rootParameters;
	signatureDesc.NumStaticSamplers = 1;
	signatureDesc.pStaticSamplers = &staticSamplerDesc;
	signatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* serializedRootSig = 0;
	ID3DBlob* errorBlob = 0;
	ThrowIfFailed(D3D12SerializeRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &serializedRootSig, &errorBlob));
	ThrowIfFailed(device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&modelRootSignature)));

	if (serializedRootSig)
	{
		serializedRootSig->Release();
	}
	if (errorBlob)
	{
		errorBlob->Release();
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

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
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

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].InputSlot = 0;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

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
	desc.Width = Utils::Align(sizeof(TransformBuffer), 256);
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

void Dx12GlobalContext::CreateLightBuffer()
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = Utils::Align(sizeof(PhongBuffer), 256);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	lightBuffer = CreateResource(&bufferPlacement, &desc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 0);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = lightBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = desc.Width;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {};
	DescriptorAllocate(&shaderDescriptorHeap, &cpuDescriptor, 0);
	device->CreateConstantBufferView(&cbvDesc, cpuDescriptor);
}

void Dx12GlobalContext::Run(std::vector<std::pair<std::string, std::string>> modelTexturePaths)
{
	isRunning = true;

	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency(&timerFrequency);

	LARGE_INTEGER beginTime;
	QueryPerformanceCounter(&beginTime);

	std::vector<Dx12Model> models;
	for (const auto& [modelPath, texturePath] : modelTexturePaths)
	{
		models.emplace_back(ModelLoader::LoadModelFromFile(modelPath, texturePath));
	}

	ThrowIfFailed(commandList->Close());
	commandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&commandList);

	WaitForGpu();

	ThrowIfFailed(commandAllocator->Reset());
	ThrowIfFailed(commandList->Reset(commandAllocator, 0));

	ClearUploadBuffer(&uploadBuffer);

	f32 Time = 0.0f;

	while (isRunning)
	{
		LARGE_INTEGER endTime;
		QueryPerformanceCounter(&endTime);
		f32 frameTime = static_cast<f32>(endTime.QuadPart - beginTime.QuadPart) / static_cast<f32>(timerFrequency.QuadPart);
		beginTime = endTime;

		frameTimeLogger.LogFrameTime(frameTime);

		M4 WTransform = M4::Translation(0.0f, 0.0f, 1.0f) * M4::Rotation(0.0f, 0.0f, 0.0f) * M4::Scale(0.1f, 0.1f, 0.1f);

		M4 WVPTransform = (M4::Perspective(Utils::DegreesToRadians(90.0f), aspectRatio, 0.01f, 1000.0f) * camera.getCameraTransformMatrix() * WTransform);

		TransformBuffer transformBufferCPU = {};
		transformBufferCPU.WTransform = WTransform;
		transformBufferCPU.WVPTransform = WVPTransform;
		transformBufferCPU.NormalWTransform = WTransform.Inverse();
		transformBufferCPU.Shininess = 4.0f;
		transformBufferCPU.SpecularStrength = 1.0f;

		CopyDataToBuffer(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &transformBufferCPU, sizeof(transformBufferCPU), transformBuffer);
				
		Time += frameTime;
		if (Time > 2.0f * Constants::PI)
		{
			Time -= 2.0f * Constants::PI;
		}

		PhongBuffer PhongBufferCPU = {};
		PhongBufferCPU.LightAmbientIntensity = 0.4f;
		PhongBufferCPU.LightColor = V3(1.0f, 0.9f, 0.1f);
		PhongBufferCPU.LightDirection = V3::Normalize(V3(-1.0f, cos(Time), 0.0f));
		PhongBufferCPU.CameraPos = camera.getPosition();
		CopyDataToBuffer(
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			&PhongBufferCPU,
			sizeof(PhongBufferCPU),
			lightBuffer);

		TransitionResource(frameBuffers[currentFrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);		

		const FLOAT color[4] = { 1, 0, 1, 1 };
		commandList->ClearRenderTargetView(frameBufferDescriptors[currentFrameIndex], color, 0, nullptr);
		commandList->ClearDepthStencilView(depthDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);

		commandList->OMSetRenderTargets(1, frameBufferDescriptors + currentFrameIndex, 0, &depthDescriptor);

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = frameBufferWidthF32;
		viewport.Height = frameBufferHeightF32;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		commandList->RSSetViewports(1, &viewport);

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.right = frameBufferWidth;
		scissorRect.top = 0;
		scissorRect.bottom = frameBufferHeight;
		commandList->RSSetScissorRects(1, &scissorRect);

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetGraphicsRootSignature(modelRootSignature);
		commandList->SetPipelineState(modelPipeline);
		commandList->SetDescriptorHeaps(1, &shaderDescriptorHeap.heap);
		commandList->SetGraphicsRootDescriptorTable(1, transformDescriptor);

		for (Dx12Model& model : models)
		{
			D3D12_VERTEX_BUFFER_VIEW vbv = {};
			vbv.BufferLocation = model.GetVertexBuffer()->GetGPUVirtualAddress();
			vbv.SizeInBytes = sizeof(Vertex) * model.GetVertexCount();
			vbv.StrideInBytes = sizeof(Vertex);
			commandList->IASetVertexBuffers(0, 1, &vbv);

			D3D12_INDEX_BUFFER_VIEW ibv = {};
			ibv.BufferLocation = model.GetIndexBuffer()->GetGPUVirtualAddress();
			ibv.SizeInBytes = sizeof(u32) * model.GetIndexCount();
			ibv.Format = DXGI_FORMAT_R32_UINT;
			commandList->IASetIndexBuffer(&ibv);

			const auto& meshes = model.GetMeshes();
			const auto& textureDescriptors = model.GetTextureDescriptors();

			for (const Mesh& mesh : meshes)
			{
				commandList->SetGraphicsRootDescriptorTable(0, textureDescriptors[mesh.textureId]);
				commandList->DrawIndexedInstanced(mesh.indexCount, 1, mesh.indexOffset, mesh.vertexOffset, 0);
			}
		}

		TransitionResource(frameBuffers[currentFrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		ThrowIfFailed(commandList->Close());
		commandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&commandList);
		swapChain->Present(enableVSync ? 1 : 0, 0);

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

ID3D12Resource* Dx12GlobalContext::CreateBufferAsset(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, const void* bufferData)
{
	return activeInstance ? static_cast<Dx12GlobalContext*>(activeInstance)->CreateBufferAssetInternal(desc, initialState, bufferData) : nullptr;
}

ID3D12Resource* Dx12GlobalContext::CreateBufferAssetInternal(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, const void* bufferData)
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

	u64 uploadSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT mipFootPrints[Constants::MAX_MIP_LEVELS] = {};
	device->GetCopyableFootprints(desc, 0, desc->MipLevels, 0, mipFootPrints, 0, 0, &uploadSize);

	u64 uploadOffset = 0;
	u8* uploadTexels = UploadArenaPushSize(&uploadBuffer, uploadSize, &uploadOffset);

	u64 bytesPerPixel = Utils::Dx12GetBytesPerPixel(desc->Format);

	RGBA8* mipMemory = desc->MipLevels > 1 ? new RGBA8[uploadSize] : nullptr;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* currFootPrint = mipFootPrints + 0;

	currFootPrint->Offset += uploadOffset;
	for (u32 y = 0; y < desc->Height; ++y)
	{
		u8* src = (u8*)texels + (y * currFootPrint->Footprint.Width) * bytesPerPixel;
		u8* dest = uploadTexels + (y * currFootPrint->Footprint.RowPitch);
		memcpy(dest, src, bytesPerPixel * currFootPrint->Footprint.Width);
	}

	RGBA8* srcMipStart = mipMemory - mipFootPrints[0].Footprint.Width * mipFootPrints[0].Footprint.Height;
	RGBA8* dstMipStart = mipMemory;
	for (u32 mipId = 1; mipId < desc->MipLevels; ++mipId)
	{
		Assert(desc->Format == DXGI_FORMAT_R8G8B8A8_UNORM);
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* prevFootPrint = mipFootPrints + mipId - 1;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* currFootPrint = mipFootPrints + mipId;

		RGBA8* srcTexelBase = mipId == 1 ? (RGBA8*)texels : srcMipStart;
		for (u32 Y = 0; Y < currFootPrint->Footprint.Height; ++Y)
		{
			for (u32 X = 0; X < currFootPrint->Footprint.Width; ++X)
			{
				RGBA8* dstTexel = dstMipStart + Y * currFootPrint->Footprint.Width + X;

				RGBA8* srcTexel00 = srcTexelBase + (2 * Y + 0) * prevFootPrint->Footprint.Width + 2 * X + 0;
				RGBA8* srcTexel01 = srcTexelBase + (2 * Y + 0) * prevFootPrint->Footprint.Width + 2 * X + 1;
				RGBA8* srcTexel10 = srcTexelBase + (2 * Y + 1) * prevFootPrint->Footprint.Width + 2 * X + 0;
				RGBA8* srcTexel11 = srcTexelBase + (2 * Y + 1) * prevFootPrint->Footprint.Width + 2 * X + 1;

				dstTexel->Red = u8(round(f32(srcTexel00->Red + srcTexel01->Red + srcTexel10->Red + srcTexel11->Red) / 4.0f));
				dstTexel->Green = u8(round(f32(srcTexel00->Green + srcTexel01->Green + srcTexel10->Green + srcTexel11->Green) / 4.0f));
				dstTexel->Blue = u8(round(f32(srcTexel00->Blue + srcTexel01->Blue + srcTexel10->Blue + srcTexel11->Blue) / 4.0f));
				dstTexel->Alpha = u8(round(f32(srcTexel00->Alpha + srcTexel01->Alpha + srcTexel10->Alpha + srcTexel11->Alpha) / 4.0f));
			}
		}

		RGBA8* srcRowY = dstMipStart;
		u8* dstRowY = uploadTexels + currFootPrint->Offset;
		currFootPrint->Offset += uploadOffset;

		for (u32 Y = 0; Y < currFootPrint->Footprint.Height; ++Y)
		{
			memcpy(dstRowY, srcRowY, bytesPerPixel * currFootPrint->Footprint.Width);

			dstRowY += currFootPrint->Footprint.RowPitch;
			srcRowY += currFootPrint->Footprint.Width;
		}

		srcMipStart += prevFootPrint->Footprint.Width * prevFootPrint->Footprint.Height;
		dstMipStart += currFootPrint->Footprint.Width * currFootPrint->Footprint.Height;
	}

	if (mipMemory)
	{
		delete(mipMemory);
	}

	ID3D12Resource* result = CreateResource(&texturePlacement, desc, D3D12_RESOURCE_STATE_COPY_DEST, 0);

	for (u32 mipId = 0; mipId < desc->MipLevels; ++mipId)
	{
		D3D12_TEXTURE_COPY_LOCATION sourceRegion = {};
		sourceRegion.pResource = uploadBuffer.gpuBuffer;
		sourceRegion.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		sourceRegion.PlacedFootprint = mipFootPrints[mipId];

		D3D12_TEXTURE_COPY_LOCATION destinationRegion = {};
		destinationRegion.pResource = result;
		destinationRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destinationRegion.SubresourceIndex = mipId;

		commandList->CopyTextureRegion(&destinationRegion, 0, 0, 0, &sourceRegion, nullptr);
	}

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
	desc.Texture2D.MipLevels = gpuTexture->GetDesc().MipLevels;
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
