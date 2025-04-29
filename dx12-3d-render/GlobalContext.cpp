#include "GlobalContext.h"
#include "Model.h"

static LRESULT CALLBACK Win32WindowCallBack(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	case WM_CLOSE:
		GlobalContext::Stop();
		return 0;
	default:
		return DefWindowProcA(windowHandle, message, wParam, lParam);
	}
}

const f32 GlobalContext::pi(std::numbers::pi_v<f32>);
bool GlobalContext::isRunning(false);

GlobalContext::GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height)
{
	WNDCLASSA windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Win32WindowCallBack;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = windowTitle;
	samplerType = SamplerType::BilinearFiltration;
	borderColor = Utils::u32ColorToV3Rgb(Colors::Black);

	if (!RegisterClassA(&windowClass)) AssertMsg("Failed to register class!");

	windowHandle = CreateWindowExA(
		0,
		windowClass.lpszClassName,
		windowTitle,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		//WS_POPUP | WS_VISIBLE,
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

	frameBufferPixels = new u32[frameBufferWidth * frameBufferHeight];
	zBuffer = new f32[frameBufferWidth * frameBufferHeight];
}

GlobalContext::~GlobalContext()
{
	ReleaseResources();
}

void GlobalContext::Run()
{
	isRunning = true;

	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency(&timerFrequency);

	LARGE_INTEGER beginTime;
	QueryPerformanceCounter(&beginTime);

	const f32 speed = 0.75f;
	f32 currentTime = -2.0f * pi;

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
		if (currentTime > 2.0f * pi)
		{
			currentTime -= 2.0f * pi;
		}

		Model cube;
		cube.LoadCube();

		M4 transform = (M4::Perspective(aspectRatio, 1.57f, 0.01f, 1000.0f) * camera.getCameraTransformMatrix() * M4::Translation(0, 0, 3) * M4::Rotation(currentTime, 0, 0) * M4::Scale(1, 1, 1));

		for (u32 i = 0; i < cube.indices.size(); i += 3)
		{
			u32 Index0 = cube.indices[i + 0];
			u32 Index1 = cube.indices[i + 1];
			u32 Index2 = cube.indices[i + 2];

			DrawTriangle(
				cube.vertices[Index0], cube.vertices[Index1], cube.vertices[Index2],
				cube.uvs[Index0], cube.uvs[Index1], cube.uvs[Index2],
				transform, *cube.texture);
		}

		camera.UpdateMouseControl(windowHandle);
		camera.UpdateViewMatrix(frameTime, wButtonPressed, aButtonPressed, sButtonPressed, dButtonPressed);

		ProcessSystemMessages();
		RenderFrame();
	}
}

void GlobalContext::Stop()
{
	isRunning = false;
}

void GlobalContext::ReleaseResources()
{
	if (deviceContext && windowHandle)
		ReleaseDC(windowHandle, deviceContext);
	if (frameBufferPixels)
	{
		delete[] frameBufferPixels;
		frameBufferPixels = nullptr;
	}
	if (zBuffer)
	{
		delete[] zBuffer;
		zBuffer = nullptr;
	}
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
		frameBufferPixels,
		&bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	));
}

V2f GlobalContext::NdcToBufferCoordinates(V2f NdcPoint) const
{
	return V2f(frameBufferWidthF32, frameBufferHeightF32) * 0.5f * (NdcPoint + V2f(1.0f, 1.0f));
}

void GlobalContext::DrawTriangle(const V3& modelVertex0, const V3& modelVertex1, const V3& modelVertex2, const V2f& modelUv0, const V2f& modelUv1, const V2f& modelUv2, const M4& transform, const Texture& texture) const
{
	V4 transformedPoint0 = transform * V4(modelVertex0, 1.0f);
	V4 transformedPoint1 = transform * V4(modelVertex1, 1.0f);
	V4 transformedPoint2 = transform * V4(modelVertex2, 1.0f);

	transformedPoint0.xyz /= transformedPoint0.w;
	transformedPoint1.xyz /= transformedPoint1.w;
	transformedPoint2.xyz /= transformedPoint2.w;

	V2f pointA = NdcToBufferCoordinates(transformedPoint0.xy);
	V2f pointB = NdcToBufferCoordinates(transformedPoint1.xy);
	V2f pointC = NdcToBufferCoordinates(transformedPoint2.xy);

	i32 minX = (i32)min(min(pointA.x, pointB.x), pointC.x);
	i32 maxX = (i32)ceil(max(max(pointA.x, pointB.x), pointC.x));
	i32 minY = (i32)min(min(pointA.y, pointB.y), pointC.y);
	i32 maxY = (i32)ceil(max(max(pointA.y, pointB.y), pointC.y));

	minX = std::clamp(minX, 0, (i32)frameBufferWidth - 1);
	maxX = std::clamp(maxX, 0, (i32)frameBufferWidth - 1);
	minY = std::clamp(minY, 0, (i32)frameBufferHeight - 1);
	maxY = std::clamp(maxY, 0, (i32)frameBufferHeight - 1);

	V2f edges[] =
	{
		pointB - pointA,
		pointC - pointB,
		pointA - pointC
	};

	bool isTopLeft[] =
	{
		(edges[0].x >= 0.0f && edges[0].y > 0.0f) || (edges[0].x > 0.0f && edges[0].y == 0.0f),
		(edges[1].x >= 0.0f && edges[1].y > 0.0f) || (edges[1].x > 0.0f && edges[1].y == 0.0f),
		(edges[2].x >= 0.0f && edges[2].y > 0.0f) || (edges[2].x > 0.0f && edges[2].y == 0.0f)
	};

	f32 barycentricDiv = V2f::CrossProduct(pointB - pointA, pointC - pointA);

	for (i32 Y = minY; Y <= maxY; ++Y)
	{
		for (i32 X = minX; X <= maxX; ++X)
		{
			V2f pixelPoint = V2f((f32)X, (f32)Y) + V2f(0.5f, 0.5f);

			V2f pixelEdges[] =
			{
				pixelPoint - pointA,
				pixelPoint - pointB,
				pixelPoint - pointC
			};

			f32 crossLengths[] =
			{
				V2f::CrossProduct(pixelEdges[0], edges[0]),
				V2f::CrossProduct(pixelEdges[1], edges[1]),
				V2f::CrossProduct(pixelEdges[2], edges[2])
			};

			if ((crossLengths[0] > 0.0f || (isTopLeft[0] && crossLengths[0] == 0.0f)) &&
				(crossLengths[1] > 0.0f || (isTopLeft[1] && crossLengths[1] == 0.0f)) &&
				(crossLengths[2] > 0.0f || (isTopLeft[2] && crossLengths[2] == 0.0f)))
			{
				u32 pixelIndex = Y * frameBufferWidth + X;

				f32 t0 = -crossLengths[1] / barycentricDiv;
				f32 t1 = -crossLengths[2] / barycentricDiv;
				f32 t2 = -crossLengths[0] / barycentricDiv;

				f32 depth = t0 * transformedPoint0.z + t1 * transformedPoint1.z + t2 * transformedPoint2.z;
				if (depth >= 0.0f && depth <= 1.0f && depth < zBuffer[pixelIndex])
				{
					V2f uv = (t0 * modelUv0 / transformedPoint0.w) + (t1 * modelUv1 / transformedPoint1.w) + (t2 * modelUv2 / transformedPoint2.w);
					uv /= (t0 / transformedPoint0.w + t1 / transformedPoint1.w + t2 / transformedPoint2.w);

					u32 texelColor;

					switch (samplerType)
					{
					case SamplerType::NearestTexel:
					{
						i32 texelX = (i32)floorf(uv.x * i32(texture.getWidth() - 1));
						i32 texelY = (i32)floorf(uv.y * i32(texture.getHeight() - 1));

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
						
						V2i texelPos[pointsCount] =
						{
							V2i((i32)floorf(texelV2.x), (i32)floorf(texelV2.y)),
							texelPos[0] + V2i(1, 0),
							texelPos[0] + V2i(0, 1),
							texelPos[0] + V2i(1, 1),
						};

						V3 texelColors[pointsCount] = {};
						for (u32 i = 0; i < pointsCount; ++i)
						{
							V2i currentTexelPos = texelPos[i];
							if (currentTexelPos.x >= 0 && currentTexelPos.x < (i32)texture.getWidth() &&
								currentTexelPos.y >= 0 && currentTexelPos.y < (i32)texture.getHeight())
							{
								texelColors[i] = Utils::u32ColorToV3Rgb(texture[currentTexelPos.y * texture.getWidth() + currentTexelPos.x]);								
							}
							else
							{
								texelColors[i] = borderColor;
							}

							f32 s = texelV2.x - floorf(texelV2.x);
							f32 k = texelV2.y - floorf(texelV2.y);

							V3 interploatedColor0 = Utils::Lerp(texelColors[0], texelColors[1], s);
							V3 interploatedColor1 = Utils::Lerp(texelColors[2], texelColors[3], s);							
							V3 color = Utils::Lerp(interploatedColor0, interploatedColor1, k);

							texelColor = Utils::V3RgbToU32Color(color);
						}

					} break;
					}

					frameBufferPixels[pixelIndex] = texelColor;
					zBuffer[pixelIndex] = depth;
				}
			}
		}
	}
}

void GlobalContext::DrawTriangle(const V3& modelVertex0, const V3& modelVertex1, const V3& modelVertex2, const V3& modelColor0, const V3& modelColor1, const V3& modelColor2, const M4& transform) const
{
	V4 transformedPoint0 = transform * V4(modelVertex0, 1.0f);
	V4 transformedPoint1 = transform * V4(modelVertex1, 1.0f);
	V4 transformedPoint2 = transform * V4(modelVertex2, 1.0f);

	transformedPoint0.xyz /= transformedPoint0.w;
	transformedPoint1.xyz /= transformedPoint1.w;
	transformedPoint2.xyz /= transformedPoint2.w;

	V2 pointA = NdcToBufferCoordinates(transformedPoint0.xy);
	V2 pointB = NdcToBufferCoordinates(transformedPoint1.xy);
	V2 pointC = NdcToBufferCoordinates(transformedPoint2.xy);

	i32 minX = (i32)min(min(pointA.x, pointB.x), pointC.x);
	i32 maxX = (i32)ceil(max(max(pointA.x, pointB.x), pointC.x));
	i32 minY = (i32)min(min(pointA.y, pointB.y), pointC.y);
	i32 maxY = (i32)ceil(max(max(pointA.y, pointB.y), pointC.y));

	minX = std::clamp(minX, 0, (i32)frameBufferWidth - 1);
	maxX = std::clamp(maxX, 0, (i32)frameBufferWidth - 1);
	minY = std::clamp(minY, 0, (i32)frameBufferHeight - 1);
	maxY = std::clamp(maxY, 0, (i32)frameBufferHeight - 1);

	V2f edges[] =
	{
		pointB - pointA,
		pointC - pointB,
		pointA - pointC
	};

	bool isTopLeft[] =
	{
		(edges[0].x >= 0.0f && edges[0].y > 0.0f) || (edges[0].x > 0.0f && edges[0].y == 0.0f),
		(edges[1].x >= 0.0f && edges[1].y > 0.0f) || (edges[1].x > 0.0f && edges[1].y == 0.0f),
		(edges[2].x >= 0.0f && edges[2].y > 0.0f) || (edges[2].x > 0.0f && edges[2].y == 0.0f)
	};

	f32 barycentricDiv = V2f::CrossProduct(pointB - pointA, pointC - pointA);

	for (i32 Y = minY; Y <= maxY; ++Y)
	{
		for (i32 X = minX; X <= maxX; ++X)
		{
			V2f pixelPoint = V2((f32)X, (f32)Y) + V2(0.5f, 0.5f);

			V2f pixelEdges[] =
			{
				pixelPoint - pointA,
				pixelPoint - pointB,
				pixelPoint - pointC
			};

			f32 crossLengths[] =
			{
				V2f::CrossProduct(pixelEdges[0], edges[0]),
				V2f::CrossProduct(pixelEdges[1], edges[1]),
				V2f::CrossProduct(pixelEdges[2], edges[2])
			};

			if ((crossLengths[0] > 0.0f || (isTopLeft[0] && crossLengths[0] == 0.0f)) &&
				(crossLengths[1] > 0.0f || (isTopLeft[1] && crossLengths[1] == 0.0f)) &&
				(crossLengths[2] > 0.0f || (isTopLeft[2] && crossLengths[2] == 0.0f)))
			{
				u32 pixelIndex = Y * frameBufferWidth + X;

				f32 t0 = -crossLengths[1] / barycentricDiv;
				f32 t1 = -crossLengths[2] / barycentricDiv;
				f32 t2 = -crossLengths[0] / barycentricDiv;

				f32 depth = t0 * transformedPoint0.z + t1 * transformedPoint1.z + t2 * transformedPoint2.z;
				if (depth >= 0.0f && depth <= 1.0f && depth < zBuffer[pixelIndex])
				{
					V3 finalColor = (t0 * modelColor0 + t1 * modelColor1 + t2 * modelColor2) * 255.0f;
					frameBufferPixels[pixelIndex] = ((u32)0xFF << 24) | ((u32)finalColor.r << 16) | ((u32)finalColor.g << 8) | (u32)finalColor.b;

					zBuffer[pixelIndex] = depth;
				}
			}
		}
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

u32* GlobalContext::GetFrameBufferPixels() const
{
	return frameBufferPixels;
}

f32* GlobalContext::GetZBuffer() const
{
	return zBuffer;
}
