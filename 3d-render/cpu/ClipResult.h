#pragma once

#include <vector>
#include "ClipVertex.h"
#include "Utils.h"
#include "Constants.h"

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
