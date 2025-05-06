#include "ClipVertex.h"

bool ClipVertex::IsBehindPlane(ClipAxis axis) const
{
	using enum ClipAxis;

	switch (axis)
	{
	case Left:   return position.x < -position.w;
	case Right:  return position.x > position.w;
	case Top:    return position.y > position.w;
	case Bottom: return position.y < -position.w;
	case Near:   return position.z < 0;
	case Far:    return position.z > position.w;
	case W:      return position.w < Constants::W_CLIPPING_PLANE;
	default:     return false;
	}
}

ClipVertex ClipVertex::IntersectWith(const ClipVertex& other, ClipAxis axis) const
{
	ClipVertex result;
	float S = 0.0f;

	using enum ClipAxis;
	switch (axis)
	{
	case Left:   S = -(position.w + position.x) / ((other.position.x - position.x) + (other.position.w - position.w)); break;
	case Right:  S = (position.w - position.x) / ((other.position.x - position.x) - (other.position.w - position.w)); break;
	case Top:    S = (position.w - position.y) / ((other.position.y - position.y) - (other.position.w - position.w)); break;
	case Bottom: S = -(position.w + position.y) / ((other.position.y - position.y) + (other.position.w - position.w)); break;
	case Near:   S = -position.z / (other.position.z - position.z); break;
	case Far:    S = (position.w - position.z) / ((other.position.z - position.z) - (other.position.w - position.w)); break;
	case W:      S = (Constants::W_CLIPPING_PLANE - position.w) / (other.position.w - position.w); break;
	}

	result.position = (1.0f - S) * position + S * other.position;
	result.uv = (1.0f - S) * uv + S * other.uv;
	return result;
}
