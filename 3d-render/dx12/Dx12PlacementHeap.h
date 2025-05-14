#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Typedefs.h"
#include <Utils.h>

class Dx12PlacementHeap  
{  
public:  
   Dx12PlacementHeap(ID3D12Device* device, D3D12_HEAP_TYPE type, u64 size, D3D12_HEAP_FLAGS flags);  
   Dx12PlacementHeap();

   u64 size;  
   u64 used;  
   ID3D12Heap* heap;  
};
