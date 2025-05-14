#include "Dx12PlacementHeap.h"

Dx12PlacementHeap::Dx12PlacementHeap(ID3D12Device* device, D3D12_HEAP_TYPE type, u64 size, D3D12_HEAP_FLAGS flags)
{
    this->size = size;
    used = 0;

    D3D12_HEAP_DESC desc = {};
    desc.SizeInBytes = size;
    desc.Properties.Type = type;
    desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    desc.Flags = flags;

    device->CreateHeap(&desc, IID_PPV_ARGS(&heap));
}

Dx12PlacementHeap::Dx12PlacementHeap()
{
	size = 0;
	used = 0;
	heap = nullptr;
}
