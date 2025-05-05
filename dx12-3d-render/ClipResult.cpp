#include "ClipResult.h"
#include "Utils.h"
#include "Constants.h"

void ClipResult::Clear()
{
	vertices.clear();
}

void ClipResult::AddTriangle(const ClipVertex& a, const ClipVertex& b, const ClipVertex& c)
{	
	vertices.push_back(a);
	vertices.push_back(b);
	vertices.push_back(c);
}

u32 ClipResult::GetTriangleCount() const
{
	return static_cast<u32>(vertices.size()) / 3;
}

const ClipVertex& ClipResult::GetVertex(u32 index) const
{
	return vertices[index];
}

void ClipResult::ClipToAxis(const ClipAxis axis, ClipResult& output) const
{
	output.Clear();

	for (u32 i = 0; i < GetTriangleCount(); ++i)
	{
		const ClipVertex v0 = GetVertex(3 * i + 0);
		const ClipVertex v1 = GetVertex(3 * i + 1);
		const ClipVertex v2 = GetVertex(3 * i + 2);

		const ClipVertex verts[] = { v0, v1, v2 };
		const bool behind[] = { v0.IsBehindPlane(axis), v1.IsBehindPlane(axis), v2.IsBehindPlane(axis) };
		const u32 count = behind[0] + behind[1] + behind[2];

		switch (count)
		{
		case 0:
		{
			output.AddTriangle(v0, v1, v2);
			break;
		}
		case 1:
		{
			u32 clipped = behind[0] ? 0 : (behind[1] ? 1 : 2);
			u32 v0 = (clipped + 1) % 3;
			u32 v1 = (clipped + 2) % 3;

			ClipVertex i0 = verts[v0].IntersectWith(verts[clipped], axis);
			ClipVertex i1 = verts[v1].IntersectWith(verts[clipped], axis);

			output.AddTriangle(verts[v0], verts[v1], i0);
			output.AddTriangle(verts[v1], i1, i0);
			break;
		}
		case 2:
		{
			const u32 visible = (behind[0] ? (behind[1] ? 2 : 1) : 0);
			const u32 b0 = (visible + 1) % 3;
			const u32 b1 = (visible + 2) % 3;

			const ClipVertex i0 = verts[visible].IntersectWith(verts[b0], axis);
			const ClipVertex i1 = verts[visible].IntersectWith(verts[b1], axis);

			output.AddTriangle(verts[visible], i0, i1);
			break;
		}
		case 3: break;
		}
	}
}
