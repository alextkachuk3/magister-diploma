#pragma once
#include "ClipVertex.h"
#include <vector>

class ClipResult
{
private:
	std::vector<ClipVertex> vertices;

public:
	void Clear();
	void AddTriangle(const ClipVertex& a, const ClipVertex& b, const ClipVertex& c);
	void ClipToAxis(const ClipAxis axis, ClipResult& output) const;

	u32 GetTriangleCount() const;
	const ClipVertex& GetVertex(u32 index) const;
};
