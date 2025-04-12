#include <Windows.h>
#include <cmath>
#include <vector>
#include "AssertUtils.h"
#include "GraphicsContext.h"
#include "V2.h"
#include "V3.h"
#include "M4.h"

static const f32 pi = 3.14159265359f;

static GraphicsContext graphicsContext;

static LRESULT CALLBACK Win32WindowCallBack(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	case WM_CLOSE:
		graphicsContext.SetIsRunning(false);
		return 0;
	default:
		return DefWindowProcA(windowHandle, message, wParam, lParam);
	}
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	graphicsContext.Initialize(hInstance, "Render", 1920, 1080, Win32WindowCallBack);
	graphicsContext.SetIsRunning(true);

	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency(&timerFrequency);

	LARGE_INTEGER beginTime;
	QueryPerformanceCounter(&beginTime);

	const u32 blockSize = 3;
	const f32 speed = 0.75f;
	f32 currentTime = -2.0f * pi;

	while (graphicsContext.IsRunning())
	{
		LARGE_INTEGER endTime;
		QueryPerformanceCounter(&endTime);
		f32 frameTime = (f32)(endTime.QuadPart - beginTime.QuadPart) / timerFrequency.QuadPart;
		beginTime = endTime;

		u32* pixels = graphicsContext.GetFrameBufferPixels();
		f32* zBuffer = graphicsContext.GetZBuffer();
		u32 width = graphicsContext.GetFrameBufferWidth();
		u32 height = graphicsContext.GetFrameBufferHeight();

		for (u32 y = 0; y < height; y++)
		{
			for (u32 x = 0; x < width; x++)
			{
				u8 red = 0;
				u8 green = 0;
				u8 blue = 0;
				u8 alpha = 255;
				u32 color = ((u32)alpha << 24) | ((u32)red << 16) | ((u32)green << 8) | (u32)blue;

				u32 pixelIndex = y * width + x;
				pixels[pixelIndex] = color;
				zBuffer[pixelIndex] = FLT_MAX;
			}
		}

        currentTime += frameTime * speed;
        if (currentTime > 2.0f * 3.14159f)
        {
            currentTime -= 2.0f * 3.14159f;
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

        f32 offset = abs(sin(currentTime));

        M4 transform = (M4::Translation(0, 0, 3) * M4::Rotation(currentTime, currentTime, currentTime) * M4::Scale(1, 1, 1));

        for (u32 i = 0; i < 36; i += 3)
        {
            u32 Index0 = ModelIndices[i + 0];
            u32 Index1 = ModelIndices[i + 1];
            u32 Index2 = ModelIndices[i + 2];

            graphicsContext.DrawTriangle(ModelVertices[Index0], ModelVertices[Index1], ModelVertices[Index2],
                ModelColors[Index0], ModelColors[Index1], ModelColors[Index2],
                transform);
        }

		graphicsContext.ProcessSystemMessages();
		graphicsContext.RenderFrame();
	}

	graphicsContext.ReleaseResources();
	return 0;
}
