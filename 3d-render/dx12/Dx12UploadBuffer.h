#pragma once

#include <d3d12.h>
#include "Typedefs.h"

struct Dx12UploadBuffer
{
	u64 size;
	u64 used;
	ID3D12Resource* gpuBuffer;
	u8* cpuPtr;
};
