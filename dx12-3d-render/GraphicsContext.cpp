#include "GraphicsContext.h"

GraphicsContext::GraphicsContext()
	: windowHandle(nullptr),
	deviceContext(nullptr),
	frameBufferWidth(0),
	frameBufferHeight(0),
	frameBufferPixels(nullptr),
	zBuffer(nullptr),
	isRunning(false)
{
}

GraphicsContext::~GraphicsContext()
{
	ReleaseResources();
}

void GraphicsContext::Initialize(HINSTANCE hInstance, const char* windowTitle, int width, int height, WNDPROC windowCallback = DefWindowProcA)
{
	WNDCLASSA windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = windowCallback;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = windowTitle;

	if (!RegisterClassA(&windowClass)) InvalidCodePath;

	windowHandle = CreateWindowExA(
		0,
		windowClass.lpszClassName,
		windowTitle,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!windowHandle) InvalidCodePath;

	deviceContext = GetDC(windowHandle);

	RECT clientRect;
	GetClientRect(windowHandle, &clientRect);
	frameBufferWidth = clientRect.right - clientRect.left;
	frameBufferHeight = clientRect.bottom - clientRect.top;

	frameBufferPixels = new u32[frameBufferWidth * frameBufferHeight];
	zBuffer = new f32[frameBufferWidth * frameBufferHeight];
}

void GraphicsContext::ReleaseResources()
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

void GraphicsContext::ProcessSystemMessages()
{
	MSG message;
	while (PeekMessageA(&message, windowHandle, 0, 0, PM_REMOVE))
	{
		if (message.message == WM_QUIT)
			SetIsRunning(false);
		else
		{
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}
	}
}

void GraphicsContext::RenderFrame() const
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

V2 GraphicsContext::ProjectPoint(V3 pos) const
{
	return 0.5f * (pos.xy / pos.z + V2(1.0f)) * V2((f32)GetFrameBufferWidth(), (f32)GetFrameBufferHeight());
}

void GraphicsContext::DrawTriangle(const V3* points, const V3* colors) const
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

void GraphicsContext::DrawTriangle(V3 modelVertex0, V3 modelVertex1, V3 modelVertex2, V3 modelColor0, V3 modelColor1, V3 modelColor2, M4 transform) const
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

HWND GraphicsContext::GetWindowHandle() const
{
	return windowHandle;
}

void GraphicsContext::SetWindowHandle(HWND handle)
{
	windowHandle = handle;
}

HDC GraphicsContext::GetDeviceContext() const
{
	return deviceContext;
}

void GraphicsContext::SetDeviceContext(HDC context)
{
	deviceContext = context;
}

u32 GraphicsContext::GetFrameBufferWidth() const
{
	return frameBufferWidth;
}

void GraphicsContext::SetFrameBufferWidth(u32 width)
{
	frameBufferWidth = width;
}

u32 GraphicsContext::GetFrameBufferHeight() const
{
	return frameBufferHeight;
}

void GraphicsContext::SetFrameBufferHeight(u32 height)
{
	frameBufferHeight = height;
}

u32* GraphicsContext::GetFrameBufferPixels() const
{
	return frameBufferPixels;
}

f32* GraphicsContext::GetZBuffer() const
{
	return zBuffer;
}

bool GraphicsContext::IsRunning() const
{
	return isRunning;
}

void GraphicsContext::SetIsRunning(bool running)
{
	isRunning = running;
}
