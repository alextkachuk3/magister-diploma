#include "GlobalContext.h"

static LRESULT CALLBACK Win32WindowCallBack(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	case WM_CLOSE:
		GlobalContext::Stop();
		return 0;
	case WM_SIZE:
	{
		u32 newWidth = LOWORD(lParam);
		u32 newHeight = HIWORD(lParam);
		GlobalContext::Resize(newWidth, newHeight);
		return 0;
	}
	default:
		return DefWindowProcA(windowHandle, message, wParam, lParam);
	}
}

GlobalContext* GlobalContext::activeInstance(nullptr);

GlobalContext::GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height)
{
	SetActiveInstance(this);
	WNDCLASSA windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Win32WindowCallBack;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = windowTitle;
	samplerType = SamplerType::BilinearFiltration;
	//samplerType = SamplerType::NearestTexel;
	borderColor = Utils::u32ColorToV3Rgb(Colors::Black);

	if (!RegisterClassA(&windowClass)) AssertMsg("Failed to register class!");

	windowHandle = CreateWindowExA(
		0,
		windowClass.lpszClassName,
		windowTitle,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	if (!windowHandle) AssertMsg("Failed to create window");

	deviceContext = GetDC(windowHandle);

	RECT clientRect;
	Assert(GetClientRect(windowHandle, &clientRect));
	frameBufferWidth = clientRect.right - clientRect.left;
	frameBufferHeight = clientRect.bottom - clientRect.top;
	frameBufferWidthF32 = static_cast<f32>(frameBufferWidth);
	frameBufferHeightF32 = static_cast<f32>(frameBufferHeight);
	aspectRatio = f32(frameBufferWidth) / f32(frameBufferHeight);

	frameBufferPixels = std::make_unique<u32[]>(frameBufferWidth * frameBufferHeight);
	zBuffer = std::make_unique<f32[]>(frameBufferWidth * frameBufferHeight);
}

GlobalContext::~GlobalContext()
{
	ReleaseResources();
}

void GlobalContext::SetActiveInstance(GlobalContext* instance)
{
	activeInstance = instance;
}

GlobalContext* GlobalContext::GetActiveInstance()
{
	return activeInstance;
}

void GlobalContext::Run()
{
	isRunning = true;

	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency(&timerFrequency);

	LARGE_INTEGER beginTime;
	QueryPerformanceCounter(&beginTime);

	const f32 speed = 0.75f;
	f32 currentTime = -2.0f * Constants::PI;

	while (isRunning)
	{
		LARGE_INTEGER endTime;
		QueryPerformanceCounter(&endTime);
		f32 frameTime = static_cast<f32>(endTime.QuadPart - beginTime.QuadPart) / static_cast<f32>(timerFrequency.QuadPart);
		beginTime = endTime;

		char frameTimeMessage[32];
		snprintf(frameTimeMessage, sizeof(frameTimeMessage), "FrameTime: %f\n", frameTime);
		OutputDebugStringA(frameTimeMessage);

		ClearBuffers();

		currentTime += frameTime * speed;
		if (currentTime > 2.0f * Constants::PI)
		{
			currentTime -= 2.0f * Constants::PI;
		}

		Model cube;
		cube.LoadCube();

		Model duck = ModelLoader::LoadModelFromFile("./3d_models/Duck/Duck.gltf", "./3d_models/Duck/DuckCM.png");

		M4 transform = (M4::Perspective(aspectRatio, 1.57f, 0.01f, 1000.0f) * camera.getCameraTransformMatrix() * M4::Translation(0, 0, 2) * M4::Rotation(0, currentTime, 0) * M4::Scale(0.01f, 0.01f, 0.01f));

		RenderModel(duck, transform);

		camera.UpdateMouseControl(windowHandle);
		camera.UpdateViewMatrix(frameTime, wButtonPressed, aButtonPressed, sButtonPressed, dButtonPressed);

		ProcessSystemMessages();
		RenderFrame();
	}
}

void GlobalContext::Stop()
{
	if (activeInstance)
	{
		activeInstance->StopInternal();
	}
}

void GlobalContext::StopInternal()
{
	isRunning = false;
}

void GlobalContext::ReleaseResources()
{
	if (deviceContext && windowHandle)
		ReleaseDC(windowHandle, deviceContext);

	frameBufferPixels.reset();
	zBuffer.reset();
}

void GlobalContext::ProcessSystemMessages()
{
	MSG message;
	while (PeekMessageW(&message, windowHandle, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
		case WM_QUIT:
		{
			isRunning = false;
			break;
		}
		case WM_KEYDOWN:
		{
			switch (message.wParam)
			{
			case 'W':
				wButtonPressed = true;
				break;
			case 'A':
				aButtonPressed = true;
				break;
			case 'S':
				sButtonPressed = true;
				break;
			case 'D':
				dButtonPressed = true;
				break;
			case VK_ESCAPE:
				isRunning = false;
				break;
			default:
				break;
			}
			break;
		}
		case WM_KEYUP:
		{
			switch (message.wParam)
			{
			case 'W':
				wButtonPressed = false;
				break;
			case 'A':
				aButtonPressed = false;
				break;
			case 'S':
				sButtonPressed = false;
				break;
			case 'D':
				dButtonPressed = false;
				break;
			default:
				break;
			}
			break;
		}
		default:
		{
			TranslateMessage(&message);
			DispatchMessageW(&message);
			break;
		}
		}
	}
}

void GlobalContext::RenderFrame() const
{
	BITMAPINFO bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = sizeof(tagBITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = frameBufferWidth;
	bitmapInfo.bmiHeader.biHeight = frameBufferHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	Assert(StretchDIBits(
		deviceContext,
		0, 0,
		frameBufferWidth, frameBufferHeight,
		0, 0,
		frameBufferWidth, frameBufferHeight,
		frameBufferPixels.get(),
		&bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	));
}

V2f GlobalContext::NdcToBufferCoordinates(V2f NdcPoint) const
{
	return V2f(frameBufferWidthF32, frameBufferHeightF32) * 0.5f * (NdcPoint + V2f(1.0f, 1.0f));
}

void GlobalContext::RenderModel(const Model& model, const M4& modelTransform) const
{
	V4* TransformedVertices = new V4[model.vertices.size()];
	for (u32 VertexId = 0; VertexId < model.vertices.size(); ++VertexId)
	{
		TransformedVertices[VertexId] = (modelTransform * V4(model.vertices[VertexId], 1.0f));
	}

	for (size_t i = 0; i < model.indices.size(); i += 3)
	{
		u32 Index0 = model.indices[i + 0];
		u32 Index1 = model.indices[i + 1];
		u32 Index2 = model.indices[i + 2];

		DrawTriangle(
			TransformedVertices[Index0], TransformedVertices[Index1], TransformedVertices[Index2],
			model.uvs[Index0], model.uvs[Index1], model.uvs[Index2], model.texture);
	}

	delete[] TransformedVertices;
}

void GlobalContext::DrawTriangle(const V4& ModelVertex0, const V4& ModelVertex1, const V4& ModelVertex2, const V2f& ModelUv0, const V2f& ModelUv1, const V2f& ModelUv2, const Texture& Texture) const
{
	ClipResult ping, pong;

	ClipVertex v0 = { ModelVertex0, ModelUv0 };
	ClipVertex v1 = { ModelVertex1, ModelUv1 };
	ClipVertex v2 = { ModelVertex2, ModelUv2 };

	ping.AddTriangle(v0, v1, v2);

	ping.ClipToAxis(ClipAxis::Left, pong);
	pong.ClipToAxis(ClipAxis::Right, ping);
	ping.ClipToAxis(ClipAxis::Top, pong);
	pong.ClipToAxis(ClipAxis::Bottom, ping);
	ping.ClipToAxis(ClipAxis::Near, pong);
	pong.ClipToAxis(ClipAxis::Far, ping);
	ping.ClipToAxis(ClipAxis::W, pong);

	for (u32 i = 0; i < pong.GetTriangleCount(); ++i)
	{
		DrawTriangle(
			pong.GetVertex(i * 3 + 0),
			pong.GetVertex(i * 3 + 1),
			pong.GetVertex(i * 3 + 2),
			Texture);
	}
}

void GlobalContext::DrawTriangle(const ClipVertex& vertex0, const ClipVertex& vertex1, const ClipVertex& vertex2, const Texture& texture) const
{
	ClipVertex v0 = vertex0;
	ClipVertex v1 = vertex1;
	ClipVertex v2 = vertex2;

	v0.position.w = 1.0f / v0.position.w;
	v1.position.w = 1.0f / v1.position.w;
	v2.position.w = 1.0f / v2.position.w;

	v0.position.xyz *= v0.position.w;
	v1.position.xyz *= v1.position.w;
	v2.position.xyz *= v2.position.w;

	V2f pointA = NdcToBufferCoordinates(v0.position.xy);
	V2f pointB = NdcToBufferCoordinates(v1.position.xy);
	V2f pointC = NdcToBufferCoordinates(v2.position.xy);

	i32 minX = std::max(0, (i32)floorf(std::min({ pointA.x, pointB.x, pointC.x })));
	i32 maxX = std::min((i32)frameBufferWidth - 1, (i32)ceilf(std::max({ pointA.x, pointB.x, pointC.x })));
	i32 minY = std::max(0, (i32)floorf(std::min({ pointA.y, pointB.y, pointC.y })));
	i32 maxY = std::min((i32)frameBufferHeight - 1, (i32)ceilf(std::max({ pointA.y, pointB.y, pointC.y })));

	V2f edges[] =
	{
		pointB - pointA,
		pointC - pointB,
		pointA - pointC
	};

	bool isTopLeft[] = 
	{
		(edges[0].y > 0.0f) || (edges[0].x > 0.0f && edges[0].y == 0.0f),
		(edges[1].y > 0.0f) || (edges[1].x > 0.0f && edges[1].y == 0.0f),
		(edges[2].y > 0.0f) || (edges[2].x > 0.0f && edges[2].y == 0.0f)
	};

	f32 barycentricDiv = V2f::CrossProduct(pointB - pointA, pointC - pointA);

	v0.uv *= v0.position.w;
	v1.uv *= v1.position.w;
	v2.uv *= v2.position.w;

	f32 edgesDiffX[] =
	{ 
		edges[0].y,
		edges[1].y,
		edges[2].y 
	};

	f32 edgesDiffY[] = 
	{
		-edges[0].x,
		-edges[1].x,
		-edges[2].x 
	};

	f32 edgesRowY[] = 
	{
		V2f::CrossProduct(V2f(minX, minY) - pointA, edges[0]),
		V2f::CrossProduct(V2f(minX, minY) - pointB, edges[1]),
		V2f::CrossProduct(V2f(minX, minY) - pointC, edges[2])
	};

	for (i32 y = minY; y <= maxY; ++y)
	{
		f32 edgesRowX[] = { edgesRowY[0], edgesRowY[1], edgesRowY[2] };

		for (i32 x = minX; x <= maxX; ++x)
		{
			if ((edgesRowX[0] > 0.0f || (isTopLeft[0] && edgesRowX[0] == 0.0f)) &&
				(edgesRowX[1] > 0.0f || (isTopLeft[1] && edgesRowX[1] == 0.0f)) &&
				(edgesRowX[2] > 0.0f || (isTopLeft[2] && edgesRowX[2] == 0.0f)))
			{
				u32 pixelIndex = y * frameBufferWidth + x;

				f32 t0 = -edgesRowX[1] / barycentricDiv;
				f32 t1 = -edgesRowX[2] / barycentricDiv;
				f32 t2 = -edgesRowX[0] / barycentricDiv;

				f32 depth = t0 * v0.position.z + t1 * v1.position.z + t2 * v2.position.z;

				if (depth >= 0.0f && depth <= 1.0f && depth < zBuffer[pixelIndex])
				{
					f32 oneOverW = t0 * v0.position.w + t1 * v1.position.w + t2 * v2.position.w;

					V2f uv = t0 * v0.uv + t1 * v1.uv + t2 * v2.uv;
					uv /= oneOverW;

					u32 texelColor = 0;

					switch (samplerType)
					{
					case SamplerType::NearestTexel:
					{
						i32 texelX = (i32)floorf(uv.x * texture.getWidth());
						i32 texelY = (i32)floorf(uv.y * texture.getHeight());

						if (texelX >= 0 && texelX < (i32)texture.getWidth() &&
							texelY >= 0 && texelY < (i32)texture.getHeight())
						{
							texelColor = texture[texelY * texture.getWidth() + texelX];
						}
						else
						{
							texelColor = Colors::Purple;
						}
					} break;

					case SamplerType::BilinearFiltration:
					{
						V2f texelV2 = uv * V2f((f32)texture.getWidth(), (f32)texture.getHeight()) - V2f(0.5f, 0.5f);

						const int pointsCount = 4;

						V2i texelPos[pointsCount] = {
							V2i((i32)floorf(texelV2.x), (i32)floorf(texelV2.y)),
							V2i((i32)floorf(texelV2.x) + 1, (i32)floorf(texelV2.y)),
							V2i((i32)floorf(texelV2.x), (i32)floorf(texelV2.y) + 1),
							V2i((i32)floorf(texelV2.x) + 1, (i32)floorf(texelV2.y) + 1),
						};

						V3 texelColors[pointsCount] = {};
						for (u32 i = 0; i < pointsCount; ++i)
						{
							const V2i& pos = texelPos[i];
							if (pos.x >= 0 && pos.x < (i32)texture.getWidth() &&
								pos.y >= 0 && pos.y < (i32)texture.getHeight())
							{
								texelColors[i] = Utils::u32ColorToV3Rgb(texture[pos.y * texture.getWidth() + pos.x]);
							}
							else
							{
								texelColors[i] = borderColor;
							}
						}

						f32 s = texelV2.x - floorf(texelV2.x);
						f32 k = texelV2.y - floorf(texelV2.y);

						V3 interploatedColor0 = Utils::Lerp(texelColors[0], texelColors[1], s);
						V3 interploatedColor1 = Utils::Lerp(texelColors[2], texelColors[3], s);
						V3 color = Utils::Lerp(interploatedColor0, interploatedColor1, k);

						texelColor = Utils::V3RgbToU32Color(color);
					} break;
					}

					frameBufferPixels[pixelIndex] = texelColor;
					zBuffer[pixelIndex] = depth;
				}
			}
			edgesRowX[0] += edgesDiffX[0];
			edgesRowX[1] += edgesDiffX[1];
			edgesRowX[2] += edgesDiffX[2];
		}
		edgesRowY[0] += edgesDiffY[0];
		edgesRowY[1] += edgesDiffY[1];
		edgesRowY[2] += edgesDiffY[2];
	}
}

void GlobalContext::ClearBuffers()
{
	for (u32 y = 0; y < frameBufferHeight; y++)
	{
		for (u32 x = 0; x < frameBufferWidth; x++)
		{
			u32 pixelIndex = y * frameBufferWidth + x;
			frameBufferPixels[pixelIndex] = Colors::Black;
			zBuffer[pixelIndex] = FLT_MAX;
		}
	}
}

void GlobalContext::Resize(u32 newWidth, u32 newHeight)
{
	if (activeInstance)
	{
		activeInstance->ResizeInternal(newWidth, newHeight);
	}
}

void GlobalContext::ResizeInternal(const u32 newWidth, const u32 newHeight)
{
	if (newWidth == 0 || newHeight == 0)
		return;

	frameBufferWidth = newWidth;
	frameBufferHeight = newHeight;
	frameBufferWidthF32 = static_cast<f32>(newWidth);
	frameBufferHeightF32 = static_cast<f32>(newHeight);
	aspectRatio = frameBufferWidthF32 / frameBufferHeightF32;

	frameBufferPixels = std::make_unique<u32[]>(newWidth * newHeight);
	zBuffer = std::make_unique<f32[]>(newWidth * newHeight);
}

HWND GlobalContext::GetWindowHandle() const
{
	return windowHandle;
}

void GlobalContext::SetWindowHandle(HWND handle)
{
	windowHandle = handle;
}

HDC GlobalContext::GetDeviceContext() const
{
	return deviceContext;
}

void GlobalContext::SetDeviceContext(HDC context)
{
	deviceContext = context;
}

u32 GlobalContext::GetFrameBufferWidth() const
{
	return frameBufferWidth;
}

void GlobalContext::SetFrameBufferWidth(u32 width)
{
	frameBufferWidth = width;
}

u32 GlobalContext::GetFrameBufferHeight() const
{
	return frameBufferHeight;
}

void GlobalContext::SetFrameBufferHeight(u32 height)
{
	frameBufferHeight = height;
}
