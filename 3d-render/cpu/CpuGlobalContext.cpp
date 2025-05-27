#include "CpuGlobalContext.h"

CpuGlobalContext::CpuGlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height) : GlobalContext(hInstance, windowTitle, width, height)
{
	deviceContext = GetDC(windowHandle);

	RECT clientRect;
	Assert(GetClientRect(windowHandle, &clientRect));
	frameBufferWidth = clientRect.right - clientRect.left;
	frameBufferHeight = clientRect.bottom - clientRect.top;
	frameBufferWidthF32 = static_cast<f32>(frameBufferWidth);
	frameBufferHeightF32 = static_cast<f32>(frameBufferHeight);
	aspectRatio = frameBufferWidthF32 / frameBufferHeightF32;

	samplerType = SamplerType::BilinearFiltration;
	borderColor = Utils::u32ColorToV3Rgb(Colors::Black);
	frameBufferPixels = std::make_unique<u32[]>(frameBufferWidth * frameBufferHeight);
	zBuffer = std::make_unique<f32[]>(frameBufferWidth * frameBufferHeight);
}

void CpuGlobalContext::Run(std::vector<std::pair<std::string, std::string>> modelTexturePaths)
{
	isRunning = true;

	LARGE_INTEGER timerFrequency;
	QueryPerformanceFrequency(&timerFrequency);

	LARGE_INTEGER beginTime;
	QueryPerformanceCounter(&beginTime);

	const f32 speed = 0.75f;
	f32 currentTime = -2.0f * Constants::PI;

	Model fox = ModelLoader::LoadModelFromFile("./assets/fox/Fox.gltf", "./assets/fox");
	Model sponza = ModelLoader::LoadModelFromFile("./assets/sponza/Sponza.gltf", "./assets/sponza/textures/");
	Model cube = Model::CreateCube();
	
	std::vector<Model> models;
	
	models.push_back(sponza);

	while (isRunning)
	{
		LARGE_INTEGER endTime;
		QueryPerformanceCounter(&endTime);
		f32 frameTime = static_cast<f32>(endTime.QuadPart - beginTime.QuadPart) / static_cast<f32>(timerFrequency.QuadPart);
		beginTime = endTime;

		frameTimeLogger.LogFrameTime(frameTime);

		ClearBuffers();

		currentTime += frameTime * speed;
		if (currentTime > 2.0f * Constants::PI)
		{
			currentTime -= 2.0f * Constants::PI;
		}

		M4 transform = M4::Perspective(aspectRatio, 1.57f, 0.01f, 4000.0f) * camera.getCameraTransformMatrix() * M4::Translation(0, 0, 10) * M4::Rotation(0, 0, 0) * M4::Scale(1.0f, 1.0f, 1.0f);

		for(const auto& model : models)
		{
			RenderModel(model, transform);
		}

		camera.UpdateMouseControl(windowHandle);
		camera.UpdateViewMatrix(frameTime, wButtonPressed, aButtonPressed, sButtonPressed, dButtonPressed);

		ProcessSystemMessages();
		RenderFrame();
	}
}

void CpuGlobalContext::ReleaseResources()
{
	GlobalContext::ReleaseResources();

	frameBufferPixels.reset();
	zBuffer.reset();
}

void CpuGlobalContext::RenderFrame() const
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
		0,
		0,
		frameBufferWidth,
		frameBufferHeight,
		0,
		0,
		frameBufferWidth,
		frameBufferHeight,
		frameBufferPixels.get(),
		&bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	));
}

void CpuGlobalContext::RenderModel(const Model& scene, const M4& modelTransform) const
{
	for (const auto& mesh : scene.meshes)
	{
		const Vertex* vertexData = scene.vertices.data() + mesh.vertexOffset;
		const u32* indexData = scene.indices.data() + mesh.indexOffset;
		const Texture& texture = scene.textures[mesh.textureId];

		std::vector<V4> transformedVertices(mesh.vertexCount);
		for (u32 i = 0; i < mesh.vertexCount; ++i)
		{
			transformedVertices[i] = modelTransform * V4(vertexData[i].position, 1.0f);
		}

		for (u32 i = 0; i < mesh.indexCount; i += 3)
		{
			u32 i0 = indexData[i + 0];
			u32 i1 = indexData[i + 1];
			u32 i2 = indexData[i + 2];

			const V4& v0 = transformedVertices[i0];
			const V4& v1 = transformedVertices[i1];
			const V4& v2 = transformedVertices[i2];

			const V2f& uv0 = vertexData[i0].uv;
			const V2f& uv1 = vertexData[i1].uv;
			const V2f& uv2 = vertexData[i2].uv;

			DrawTriangle(v0, v1, v2, uv0, uv1, uv2, texture);
		}
	}
}

void CpuGlobalContext::DrawTriangle(const V4& ModelVertex0, const V4& ModelVertex1, const V4& ModelVertex2, const V2f& ModelUv0, const V2f& ModelUv1, const V2f& ModelUv2, const Texture& Texture) const
{
	ClipResult ping;
	ClipResult pong;

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
		DrawTriangle(pong.GetVertex(i * 3 + 0), pong.GetVertex(i * 3 + 1), pong.GetVertex(i * 3 + 2), Texture);
	}
}

void CpuGlobalContext::DrawTriangle(const ClipVertex& vertex0, const ClipVertex& vertex1, const ClipVertex& vertex2, const Texture& texture) const
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

	i32 minX = std::max(0, static_cast<i32>(floorf(std::min({ pointA.x, pointB.x, pointC.x }))));
	i32 maxX = std::min(static_cast<i32>(frameBufferWidth - 1), static_cast<i32>(ceilf(std::max({ pointA.x, pointB.x, pointC.x }))));
	i32 minY = std::max(0, static_cast<i32>(floorf(std::min({ pointA.y, pointB.y, pointC.y }))));
	i32 maxY = std::min(static_cast<i32>(frameBufferHeight - 1), static_cast<i32>(ceilf(std::max({ pointA.y, pointB.y, pointC.y }))));

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
		V2f::CrossProduct(V2f(static_cast<f32>(minX), static_cast<f32>(minY)) - pointA, edges[0]),
		V2f::CrossProduct(V2f(static_cast<f32>(minX), static_cast<f32>(minY)) - pointB, edges[1]),
		V2f::CrossProduct(V2f(static_cast<f32>(minX), static_cast<f32>(minY)) - pointC, edges[2])
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

				f32 depth = v0.position.z + t1 * (v1.position.z - v0.position.z) + t2 * (v2.position.z - v0.position.z);

				if (depth >= 0.0f && depth <= 1.0f && depth < zBuffer[pixelIndex])
				{
					f32 oneOverW = t0 * v0.position.w + t1 * v1.position.w + t2 * v2.position.w;

					V2f uv = t0 * v0.uv + t1 * v1.uv + t2 * v2.uv;
					uv /= oneOverW;

					uv.x = uv.x - floorf(uv.x);
					uv.y = uv.y - floorf(uv.y);

					u32 texelColor = 0;

					switch (samplerType)
					{
					case SamplerType::NearestTexel:
					{
						i32 texelX = static_cast<i32>(floorf(uv.x * texture.getWidth()));
						i32 texelY = static_cast<i32>(floorf(uv.y * texture.getHeight()));

						if (texelX >= 0 && texelX < static_cast<i32>(texture.getWidth()) &&
							texelY >= 0 && texelY < static_cast<i32>(texture.getHeight()))
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
							V2i(static_cast<i32>(floorf(texelV2.x)), static_cast<i32>(floorf(texelV2.y))),
							V2i(static_cast<i32>(floorf(texelV2.x)) + 1, static_cast<i32>(floorf(texelV2.y))),
							V2i(static_cast<i32>(floorf(texelV2.x)), static_cast<i32>(floorf(texelV2.y)) + 1),
							V2i(static_cast<i32>(floorf(texelV2.x)) + 1, static_cast<i32>(floorf(texelV2.y)) + 1),
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

						texelColor = Utils::V3BgrToU32Color(color);
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

void CpuGlobalContext::ClearBuffers()
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

void CpuGlobalContext::ResizeInternal(const u32 newWidth, const u32 newHeight)
{
	GlobalContext::ResizeInternal(newWidth, newHeight);
	frameBufferPixels = std::make_unique<u32[]>(newWidth * newHeight);
	zBuffer = std::make_unique<f32[]>(newWidth * newHeight);
}
