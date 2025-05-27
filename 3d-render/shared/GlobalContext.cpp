#include "GlobalContext.h"
#include <Dx12GlobalContext.h>

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
}

GlobalContext::~GlobalContext()
{
	if (deviceContext)
	{
		ReleaseDC(windowHandle, deviceContext);
		deviceContext = nullptr;
	}
	if (windowHandle)
	{
		DestroyWindow(windowHandle);
		windowHandle = nullptr;
	}
}

void GlobalContext::SetActiveInstance(GlobalContext* instance)
{
	activeInstance = instance;
}

GlobalContext* GlobalContext::GetActiveInstance()
{
	return activeInstance;
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

void GlobalContext::Resize(const u32 newWidth, const u32 newHeight)
{
	if (activeInstance)
	{
		if (dynamic_cast<Dx12GlobalContext*>(activeInstance) == nullptr)
		{
			activeInstance->ResizeInternal(newWidth, newHeight);
		}		
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
}

void GlobalContext::ReleaseResources()
{
	if (deviceContext && windowHandle)
	{
		ReleaseDC(windowHandle, deviceContext);
		deviceContext = nullptr;
	}
	if (windowHandle)
	{
		DestroyWindow(windowHandle);
		windowHandle = nullptr;
	}
}

V2f GlobalContext::NdcToBufferCoordinates(const V2f NdcPoint) const
{
	return V2f(frameBufferWidthF32, frameBufferHeightF32) * 0.5f * (NdcPoint + V2f(1.0f, 1.0f));
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
