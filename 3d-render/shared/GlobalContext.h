#pragma once

#define NOMINMAX
#include <Windows.h>
#include <memory>
#include <cmath>
#include <algorithm>
#include "Colors.h"
#include "Typedefs.h"
#include "Utils.h"
#include "V2.h"
#include "V3.h"
#include "V4.h"
#include "M4.h"
#include "Camera.h"
#include "Texture.h"
#include "Constants.h"
#include "SamplerType.h"
#include "ClipAxis.h"
#include "ModelLoader.h"
#include "FrameTimeLogger.h"

class GlobalContext
{
public:
	GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height);
	virtual ~GlobalContext();

	static void SetActiveInstance(GlobalContext* instance);
	static GlobalContext* GetActiveInstance();

	virtual void Run() = 0;
	static void Stop();
	static void Resize(const u32 newWidth, const u32 newHeight);

protected:
	V2f NdcToBufferCoordinates(const V2f NdcPoint) const;

	void ProcessSystemMessages();
	void StopInternal();
	virtual void ResizeInternal(const u32 newWidth, const u32 newHeight);
	virtual void ReleaseResources();

	HWND windowHandle;
	HDC deviceContext;
	u32 frameBufferWidth;
	u32 frameBufferHeight;
	f32 frameBufferWidthF32;
	f32 frameBufferHeightF32;
	f32 aspectRatio;

	Camera camera;
	FrameTimeLogger frameTimeLogger;

	static GlobalContext* activeInstance;

	bool wButtonPressed = false;
	bool aButtonPressed = false;
	bool sButtonPressed = false;
	bool dButtonPressed = false;
	bool leftMouseButtonPressed = false;
	bool isRunning;
};
