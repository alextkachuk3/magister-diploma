#include "GlobalContext.h"

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

	if (!RegisterClassA(&windowClass)) AssertMsg("Failed to register class!");

	windowHandle = CreateWindowExA(
		0,
		windowClass.lpszClassName,
		windowTitle,
		WS_POPUP | WS_VISIBLE,
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
		f32 frameTime = (f32)(endTime.QuadPart - beginTime.QuadPart) / timerFrequency.QuadPart;
		beginTime = endTime;

		char frameTimeMessage[64];
		snprintf(frameTimeMessage, sizeof(frameTimeMessage), "FrameTime: %f\n", frameTime);
		OutputDebugStringA(frameTimeMessage);

		for (u32 y = 0; y < frameBufferHeight; y++)
		{
			for (u32 x = 0; x < frameBufferWidth; x++)
			{
				u8 red = 0;
				u8 green = 0;
				u8 blue = 0;
				u8 alpha = 255;
				u32 color = ((u32)alpha << 24) | ((u32)red << 16) | ((u32)green << 8) | (u32)blue;

				u32 pixelIndex = y * frameBufferWidth + x;
				frameBufferPixels[pixelIndex] = color;
				zBuffer[pixelIndex] = FLT_MAX;
			}
		}

		currentTime += frameTime * speed;
		if (currentTime > 2.0f * pi)
		{
			currentTime -= 2.0f * pi;
		}

		V3 ModelVertices[] =
		{
			V3(-0.5f, -0.5f, -0.5f),
			V3(-0.5f, 0.5f, -0.5f),
			V3(0.5f, 0.5f, -0.5f),
			V3(0.5f, -0.5f, -0.5f),
			V3(-0.5f, -0.5f, 0.5f),
			V3(-0.5f, 0.5f, 0.5f),
			V3(0.5f, 0.5f, 0.5f),
			V3(0.5f, -0.5f, 0.5f),
		};

		V3 ModelColors[] =
		{
			V3(1, 0, 0),
			V3(0, 0, 1),
			V3(0.2f, 0.8f, 0.2f),
			V3(1, 0, 1),
			V3(1, 1, 0),
			V3(1, 0.5f, 0),
			V3(0.5f, 0, 0.5f),
			V3(0.2f, 0.2f, 1),
		};

		u32 ModelIndices[] =
		{
			0, 1, 2,
			2, 3, 0,

			6, 5, 4,
			4, 7, 6,

			4, 5, 1,
			1, 0, 4,

			3, 2, 6,
			6, 7, 3,

			1, 5, 6,
			6, 2, 1,

			4, 0, 3,
			3, 7, 4,
		};

		M4 transform = (camera.getTranlationMatrix() * M4::Translation(0, 0, 3) * M4::Rotation(currentTime, currentTime, currentTime) * M4::Scale(1, 1, 1));

		for (u32 i = 0; i < 36; i += 3)
		{
			u32 Index0 = ModelIndices[i + 0];
			u32 Index1 = ModelIndices[i + 1];
			u32 Index2 = ModelIndices[i + 2];

			DrawTriangle(ModelVertices[Index0], ModelVertices[Index1], ModelVertices[Index2],
				ModelColors[Index0], ModelColors[Index1], ModelColors[Index2],
				transform);
		}

		M4 cameraTransform = M4::Identity();

		if (wDown)
		{
			camera.move(V3::LookAt() * frameTime);
		}
		if (aDown)
		{
			camera.moveReverse(V3::Right() * frameTime);
		}
		if (sDown)
		{
			camera.moveReverse(V3::LookAt() * frameTime);
		}
		if (dDown)
		{
			camera.move(V3::Right() * frameTime);
		}

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
			u64 keyCode = message.wParam;
			bool isDown = !((message.lParam >> 31) & 0x1);

			switch (keyCode)
			{
			case 'W':
				wDown = isDown;
				break;
			case 'A':
				aDown = isDown;
				break;
			case 'S':
				sDown = isDown;
				break;
			case 'D':
				dDown = isDown;
				break;
			default:
				break;
			}
			break;
		}
		case WM_KEYUP:
		{
			u64 keyCode = message.wParam;

			switch (keyCode)
			{
			case 'W':
				wDown = false;
				break;
			case 'A':
				aDown = false;
				break;
			case 'S':
				sDown = false;
				break;
			case 'D':
				dDown = false;
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
	RECT clientRect = {};
	Assert(GetClientRect(windowHandle, &clientRect));
	uint32_t clientWidth = clientRect.right - clientRect.left;
	uint32_t clientHeight = clientRect.bottom - clientRect.top;

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
		clientWidth, clientHeight,
		0, 0,
		frameBufferWidth, frameBufferHeight,
		frameBufferPixels,
		&bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	));
}

V2 GlobalContext::ProjectPoint(V3 pos) const
{
	return 0.5f * (pos.xy / pos.z + V2(1.0f)) * V2((f32)GetFrameBufferWidth(), (f32)GetFrameBufferHeight());
}

void GlobalContext::DrawTriangle(const V3* points, const V3* colors) const
{
	V2 pointA = ProjectPoint(points[0]);
	V2 pointB = ProjectPoint(points[1]);
	V2 pointC = ProjectPoint(points[2]);

	i32 minX = (i32)min(min(pointA.x, pointB.x), pointC.x);
	i32 maxX = (i32)ceil(max(max(pointA.x, pointB.x), pointC.x));
	i32 minY = (i32)min(min(pointA.y, pointB.y), pointC.y);
	i32 maxY = (i32)ceil(max(max(pointA.y, pointB.y), pointC.y));

	minX = std::clamp(minX, 0, (i32)frameBufferWidth - 1);
	maxX = std::clamp(maxX, 0, (i32)frameBufferWidth - 1);
	minY = std::clamp(minY, 0, (i32)frameBufferHeight - 1);
	maxY = std::clamp(maxY, 0, (i32)frameBufferHeight - 1);

	V2 edges[] =
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

	f32 barycentricDiv = V2::CrossProduct(pointB - pointA, pointC - pointA);

	for (i32 Y = minY; Y <= maxY; ++Y)
	{
		for (i32 X = minX; X <= maxX; ++X)
		{
			V2 pixelPoint = V2((f32)X, (f32)Y) + V2(0.5f, 0.5f);

			V2 pixelEdges[] =
			{
				pixelPoint - pointA,
				pixelPoint - pointB,
				pixelPoint - pointC
			};

			f32 crossLengths[] =
			{
				V2::CrossProduct(pixelEdges[0], edges[0]),
				V2::CrossProduct(pixelEdges[1], edges[1]),
				V2::CrossProduct(pixelEdges[2], edges[2])
			};

			if ((crossLengths[0] > 0.0f || (isTopLeft[0] && crossLengths[0] == 0.0f)) &&
				(crossLengths[1] > 0.0f || (isTopLeft[1] && crossLengths[1] == 0.0f)) &&
				(crossLengths[2] > 0.0f || (isTopLeft[2] && crossLengths[2] == 0.0f)))
			{
				u32 pixelIndex = Y * frameBufferWidth + X;

				f32 t0 = -crossLengths[1] / barycentricDiv;
				f32 t1 = -crossLengths[2] / barycentricDiv;
				f32 t2 = -crossLengths[0] / barycentricDiv;

				f32 depth = 1.0f / (t0 * (1.0f / points[0].z) + t1 * (1.0f / points[1].z) + t2 * (1.0f / points[2].z));

				if (depth < zBuffer[pixelIndex])
				{
					V3 finalColor = (t0 * colors[0] + t1 * colors[1] + t2 * colors[2]) * 255.0f;
					frameBufferPixels[pixelIndex] = ((u32)0xFF << 24) | ((u32)finalColor.r << 16) | ((u32)finalColor.g << 8) | (u32)finalColor.b;

					zBuffer[pixelIndex] = depth;
				}
			}
		}
	}
}

void GlobalContext::DrawTriangle(const V3& modelVertex0, const V3& modelVertex1, const V3& modelVertex2, const V3& modelColor0, const V3& modelColor1, const V3& modelColor2, const M4& transform) const
{
	V3 transformedPoint0 = (transform * V4(modelVertex0, 1.0f)).xyz;
	V3 transformedPoint1 = (transform * V4(modelVertex1, 1.0f)).xyz;
	V3 transformedPoint2 = (transform * V4(modelVertex2, 1.0f)).xyz;

	V2 pointA = ProjectPoint(transformedPoint0);
	V2 pointB = ProjectPoint(transformedPoint1);
	V2 pointC = ProjectPoint(transformedPoint2);

	i32 minX = (i32)min(min(pointA.x, pointB.x), pointC.x);
	i32 maxX = (i32)ceil(max(max(pointA.x, pointB.x), pointC.x));
	i32 minY = (i32)min(min(pointA.y, pointB.y), pointC.y);
	i32 maxY = (i32)ceil(max(max(pointA.y, pointB.y), pointC.y));

	minX = std::clamp(minX, 0, (i32)frameBufferWidth - 1);
	maxX = std::clamp(maxX, 0, (i32)frameBufferWidth - 1);
	minY = std::clamp(minY, 0, (i32)frameBufferHeight - 1);
	maxY = std::clamp(maxY, 0, (i32)frameBufferHeight - 1);

	V2 edges[] =
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

	f32 barycentricDiv = V2::CrossProduct(pointB - pointA, pointC - pointA);

	for (i32 Y = minY; Y <= maxY; ++Y)
	{
		for (i32 X = minX; X <= maxX; ++X)
		{
			V2 pixelPoint = V2((f32)X, (f32)Y) + V2(0.5f, 0.5f);

			V2 pixelEdges[] =
			{
				pixelPoint - pointA,
				pixelPoint - pointB,
				pixelPoint - pointC
			};

			f32 crossLengths[] =
			{
				V2::CrossProduct(pixelEdges[0], edges[0]),
				V2::CrossProduct(pixelEdges[1], edges[1]),
				V2::CrossProduct(pixelEdges[2], edges[2])
			};

			if ((crossLengths[0] > 0.0f || (isTopLeft[0] && crossLengths[0] == 0.0f)) &&
				(crossLengths[1] > 0.0f || (isTopLeft[1] && crossLengths[1] == 0.0f)) &&
				(crossLengths[2] > 0.0f || (isTopLeft[2] && crossLengths[2] == 0.0f)))
			{
				u32 pixelIndex = Y * frameBufferWidth + X;

				f32 t0 = -crossLengths[1] / barycentricDiv;
				f32 t1 = -crossLengths[2] / barycentricDiv;
				f32 t2 = -crossLengths[0] / barycentricDiv;

				f32 depth = t0 * (1.0f / transformedPoint0.z) + t1 * (1.0f / transformedPoint1.z) + t2 * (1.0f / transformedPoint2.z);
				depth = 1.0f / depth;
				if (depth < zBuffer[pixelIndex])
				{
					V3 finalColor = (t0 * modelColor0 + t1 * modelColor1 + t2 * modelColor2) * 255.0f;
					frameBufferPixels[pixelIndex] = ((u32)0xFF << 24) | ((u32)finalColor.r << 16) | ((u32)finalColor.g << 8) | (u32)finalColor.b;

					zBuffer[pixelIndex] = depth;
				}
			}
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
