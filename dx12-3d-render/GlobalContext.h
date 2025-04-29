#pragma once

#include <Windows.h>
#include <cmath>
#include <algorithm>
#include <numbers>
#include "Typedefs.h"
#include "Utils.h"
#include "V2.h"
#include "V3.h"
#include "V4.h"
#include "M4.h"
#include "Camera.h"
#include "Texture.h"
#include "SamplerType.h"

class GlobalContext
{
private:
	HWND windowHandle;
	HDC deviceContext;
	u32 frameBufferWidth;
	u32 frameBufferHeight;
	f32 aspectRatio;
	u32* frameBufferPixels;
	f32* zBuffer;

	bool wButtonPressed = false;
	bool aButtonPressed = false;
	bool sButtonPressed = false;
	bool dButtonPressed = false;
	bool leftMouseButtonPressed = false;

	SamplerType samplerType;

	Camera camera;

	static bool isRunning;
	static const f32 pi;

public:
	GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height);
	~GlobalContext();

	void Run();
	static void Stop();

	void ReleaseResources();
	void ProcessSystemMessages();

	void RenderFrame() const;
	V2f NdcToBufferCoordinates(V2f NdcPoint) const;
	void DrawTriangle(const V3& modelVertex0, const V3& modelVertex1, const V3& modelVertex2, const V2f& modelUv0, const V2f& modelUv1, const V2f& modelUv2, const M4& transform, const Texture& texture) const;
	void DrawTriangle(const V3& ModelVertex0, const V3& ModelVertex1, const V3& ModelVertex2, const V3& ModelColor0, const V3& ModelColor1, const V3& ModelColor2, const M4& Transform) const;

	HWND GetWindowHandle() const;
	void SetWindowHandle(HWND handle);

	HDC GetDeviceContext() const;
	void SetDeviceContext(HDC context);

	u32 GetFrameBufferWidth() const;
	void SetFrameBufferWidth(u32 width);

	u32 GetFrameBufferHeight() const;
	void SetFrameBufferHeight(u32 height);

	u32* GetFrameBufferPixels() const;
	f32* GetZBuffer() const;
};
