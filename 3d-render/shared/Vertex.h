#pragma once

#include <V3.h>
#include <V2.h>

#pragma pack(push, 1)
struct Vertex
{
	V3 position;
	V2f uv;
};
#pragma pack(pop)
