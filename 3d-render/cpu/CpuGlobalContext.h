#pragma once

#include "GlobalContext.h"
#include "ClipResult.h"
#include "ClipVertex.h"

class CpuGlobalContext : public GlobalContext
{
private:	
	std::unique_ptr<u32[]> frameBufferPixels;
	std::unique_ptr<f32[]> zBuffer;

	SamplerType samplerType;
	V3 borderColor;
	Camera camera;

public:
	CpuGlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height);

	void Run();

	void ReleaseResources();
	void ProcessSystemMessages();

	void RenderFrame() const;
	void RenderModel(const Model& model, const M4& modelTransform) const;

	void DrawTriangle(const ClipVertex& vertex0, const ClipVertex& vertex1, const ClipVertex& vertex2, const Texture& texture) const;
	void DrawTriangle(const V4& ModelVertex0, const V4& ModelVertex1, const V4& ModelVertex2, const V2f& ModelUv0, const V2f& ModelUv1, const V2f& ModelUv2, const Texture& Texture) const;

	void ClearBuffers();

	void ResizeInternal(const u32 newWidth, const u32 newHeight) override;
};
