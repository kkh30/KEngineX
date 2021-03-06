#include "DX12API.h"


void GetHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter);


ke::renderer::dx12renderer::DX12API::DX12API():
    Win32Application(),
    m_useWarpDevice(false),
    m_fence(nullptr),
    m_fence_event(nullptr),
    m_cmd_allocator(nullptr),
    m_current_fence_value(1)
{
    m_enable_debug = true;
    Init(m_enable_debug);
}

ke::renderer::dx12renderer::DX12API::~DX12API()
{
    if (m_enable_debug)
    {
        ID3D12DebugDevice* debugInterface;
        if (m_device->QueryInterface(&debugInterface))
        {
            debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
            debugInterface->Release();
        }
    }

    if (m_device)
    {
        m_device->Release();
        m_device = nullptr;
    }
}


ComPtr<ID3D12GraphicsCommandList> ke::renderer::dx12renderer::DX12API::CreateGraphicsCommandList()
{
    ComPtr<ID3D12GraphicsCommandList> res;
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd_allocator.Get(), nullptr, IID_PPV_ARGS(&res));
    return res;
}

bool ke::renderer::dx12renderer::DX12API::FlushCmdQueue(ComPtr<ID3D12GraphicsCommandList> p_list,bool release_cmd_list)
{
    p_list->Close();
    ID3D12CommandList* lists[1] = { p_list.Get() };
    m_commandQueue->ExecuteCommandLists(1, lists);
    //if (release_cmd_list && p_list) 
    //{
    //    p_list->Release();
    //}
    return WaitForCmdQueue();

}

bool ke::renderer::dx12renderer::DX12API::WaitForCmdQueue()
{
    m_commandQueue->Signal(m_fence.Get(), m_current_fence_value);
    if (m_fence->GetCompletedValue() < m_current_fence_value) {
        //Wait for cmd queue to finish
        BlockToWait();
    }
    m_current_fence_value++;
    return true;
}

bool ke::renderer::dx12renderer::DX12API::TransitResource(
    ID3D12Resource* p_resource,
    D3D12_RESOURCE_STATES before_state,
    D3D12_RESOURCE_STATES after_state)
{
    D3D12_RESOURCE_BARRIER l_resource_barrier_desc = {};
    l_resource_barrier_desc.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    l_resource_barrier_desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    l_resource_barrier_desc.Transition.pResource = p_resource;
    l_resource_barrier_desc.Transition.StateBefore = before_state;
    l_resource_barrier_desc.Transition.StateAfter = after_state;

    auto cmd_list = CreateGraphicsCommandList();
    cmd_list->ResourceBarrier(1, &l_resource_barrier_desc);
    return FlushCmdQueue(cmd_list.Get());
}



void ke::renderer::dx12renderer::DX12API::Init(bool enable_validation)
{
    UINT dxgiFactoryFlags = 0;
    if(enable_validation)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

        D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        );
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
        );
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = SWAP_CHAIN_COUNT;
    swapChainDesc.Width = WINDOW_WIDTH;
    swapChainDesc.Height = WINDOW_HEIGHT;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(), 	// Swap chain needs the queue so that it can force a flush on it.
        m_hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    );

    // This sample does not support fullscreen transitions.
    factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
    swapChain.As(&m_swapChain);
    m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&m_cmd_allocator)
    );
    m_cmd_allocator->Reset();
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

}

void ke::renderer::dx12renderer::DX12API::BlockToWait()
{
    m_fence->SetEventOnCompletion(m_current_fence_value, m_fence_event);
    WaitForSingleObjectEx(m_fence_event, INFINITY,false);
}

void GetHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter)
{
    using Microsoft::WRL::ComPtr;

    ComPtr<IDXGIAdapter1> adapter;
    *ppAdapter = nullptr;

    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            // If you want a software adapter, pass in "/warp" on the command line.
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
    }

    *ppAdapter = adapter.Detach();
}
