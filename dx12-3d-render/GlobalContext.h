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
#include "Model.h"
#include "SamplerType.h"
#include "ClipAxis.h"
#include "Constants.h"
#include "ClipResult.h"
#include "ClipVertex.h"

class GlobalContext
{
private:
	HWND windowHandle;
	HDC deviceContext;
	u32 frameBufferWidth;
	u32 frameBufferHeight;
	f32 frameBufferWidthF32;
	f32 frameBufferHeightF32;
	f32 aspectRatio;
	std::unique_ptr<u32[]> frameBufferPixels;
	std::unique_ptr<f32[]> zBuffer;
	static GlobalContext* activeInstance;

	bool wButtonPressed = false;
	bool aButtonPressed = false;
	bool sButtonPressed = false;
	bool dButtonPressed = false;
	bool leftMouseButtonPressed = false;
	bool isRunning;

	SamplerType samplerType;
	V3 borderColor;
	Camera camera;

public:
	GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height);
	~GlobalContext();

	static void SetActiveInstance(GlobalContext* instance);
	static GlobalContext* GetActiveInstance();

	void Run();
	static void Stop();
	void StopInternal();

	void ReleaseResources();
	void ProcessSystemMessages();

	void RenderFrame() const;
	V2f NdcToBufferCoordinates(V2f NdcPoint) const;
	void RenderModel(const Model& model, const M4& modelTransform) const;

	void DrawTriangle(const ClipVertex& vertex0, const ClipVertex& vertex1, const ClipVertex& vertex2, const Texture& texture) const;
	void DrawTriangle(const V4& ModelVertex0, const V4& ModelVertex1, const V4& ModelVertex2, const V2f& ModelUv0, const V2f& ModelUv1, const V2f& ModelUv2, const Texture& Texture) const;
	
	void ClearBuffers();
	static void Resize(const u32 newWidth, const u32 newHeight);
	void ResizeInternal(u32 newWidth, u32 newHeight);

	HWND GetWindowHandle() const;
	void SetWindowHandle(HWND handle);

	HDC GetDeviceContext() const;
	void SetDeviceContext(HDC context);

	u32 GetFrameBufferWidth() const;
	void SetFrameBufferWidth(u32 width);

	u32 GetFrameBufferHeight() const;
	void SetFrameBufferHeight(u32 height);
};
