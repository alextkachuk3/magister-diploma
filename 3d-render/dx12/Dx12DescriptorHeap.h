#pragma once
#include <d3d12.h>
#include <Typedefs.h>

struct Dx12DescriptorHeap
{
	ID3D12DescriptorHeap* heap;
	u64 stepSize;
	u64 maxElements;
	u64 currentElement;
};
