#pragma once

#include "V4.h"
#include "V2.h"
#include "ClipAxis.h"
#include "Constants.h"

class ClipVertex
{
public:
	V4 position;
	V2f uv;

	ClipVertex IntersectWith(const ClipVertex& other, ClipAxis axis) const;
	bool IsBehindPlane(ClipAxis axis) const;
};
