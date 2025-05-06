#include <windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <D3dx12.h>
#include <dxgi1_6.h>
#include <tchar.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

HWND g_hWnd = nullptr;
const UINT Width = 800;
const UINT Height = 600;

ComPtr<ID3D12Device> device;
ComPtr<IDXGISwapChain3> swapChain;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12DescriptorHeap> rtvHeap;
ComPtr<ID3D12Resource> renderTargets[2];
ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12GraphicsCommandList> commandList;
ComPtr<ID3D12Fence> fence;
HANDLE fenceEvent;
UINT rtvDescriptorSize;
UINT frameIndex;
UINT64 fenceValue = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DESTROY || msg == WM_CLOSE)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void WaitForPreviousFrame()
{
    const UINT64 fenceToWaitFor = ++fenceValue;
    commandQueue->Signal(fence.Get(), fenceToWaitFor);
    if (fence->GetCompletedValue() < fenceToWaitFor)
    {
        fence->SetEventOnCompletion(fenceToWaitFor, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
    frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void InitD3D12()
{
    ComPtr<IDXGIFactory6> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    ComPtr<IDXGIAdapter1> adapter;
    factory->EnumAdapters1(0, &adapter);

    D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue));

    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.Width = Width;
    scDesc.Height = Height;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> sc1;
    factory->CreateSwapChainForHwnd(commandQueue.Get(), g_hWnd, &scDesc, nullptr, nullptr, &sc1);
    sc1.As(&swapChain);

    frameIndex = swapChain->GetCurrentBackBufferIndex();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 2;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap));

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < 2; ++i)
    {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    commandList->Close();

    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void Render()
{
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), nullptr);

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
    float clearColor[] = { 0.0f, 0.2f, 0.8f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &barrier);

    commandList->Close();
    commandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(commandList.GetAddressOf()));

    swapChain->Present(1, 0);
    WaitForPreviousFrame();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("DX12BlueWindow");
    RegisterClass(&wc);

    g_hWnd = CreateWindowEx(0, wc.lpszClassName, _T("DirectX 12 Blue Window"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Width, Height,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(g_hWnd, nCmdShow);

    InitD3D12();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Render();
        }
    }

    WaitForPreviousFrame();
    CloseHandle(fenceEvent);
    return 0;
}
