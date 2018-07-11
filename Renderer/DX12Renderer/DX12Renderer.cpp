#include "DX12Renderer.h"



namespace ke
{
    namespace renderer
    {
        namespace dx12renderer
        {


            DX12Renderer::DX12Renderer() :
                Win32Application(std::bind<void>(&DX12Renderer::Render,this)),
                m_useWarpDevice(false),
                m_frameIndex(0),
                m_rtvDescriptorSize(0),
                m_fenceEvent(nullptr)
            {
            }
            
            void DX12Renderer::Init()
            {
                InitDevice();

                InitDescHeap();

                InitFramebuffer();

                InitCmdAllocator();

                InitCmdList();

                InitSynchronization();

                ShowWindow(m_hwnd, CMD_SHOW);


            }

            void DX12Renderer::Update()
            {
                MsgLoop();
            }

            void DX12Renderer::ShutDown()
            {

                
#if defined(_DEBUG)
                ID3D12DebugDevice* debugInterface;
                if (m_device->QueryInterface(&debugInterface))
                {
                    debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                    debugInterface->Release();
                }
#endif

                if (m_device) {
                    m_device->Release();
                    m_device = nullptr;
                }
            }

            FrameGraph * DX12Renderer::GetFrameGraph()
            {
                return nullptr;
            }

            void DX12Renderer::InitDevice()
            {

                UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
                // Enable the debug layer (requires the Graphics Tools "optional feature").
                // NOTE: Enabling the debug layer after device creation will invalidate the active device.
                {
                    ComPtr<ID3D12Debug> debugController;
                    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
                    {
                        debugController->EnableDebugLayer();

                        // Enable additional debug layers.
                        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                    }
                }
#endif

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
                m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
            }

            void DX12Renderer::InitSwapChain()
            {
                //Implemented in InitDevice() maybe move to here later.
                _ASSERT(0);
            }

            void DX12Renderer::InitDescHeap()
            {
                // Describe and create a render target view (RTV) descriptor heap.
                D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
                rtvHeapDesc.NumDescriptors = SWAP_CHAIN_COUNT;
                rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

                m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            }

            void DX12Renderer::InitFramebuffer()
            {
                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

                // Create a RTV for each frame.
                for (UINT n = 0; n < SWAP_CHAIN_COUNT; n++)
                {
                    m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
                    m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
                    rtvHandle.Offset(1, m_rtvDescriptorSize);
                }
            }

            void DX12Renderer::InitCmdList()
            {
                // Create the command list.
                m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));

                // Command lists are created in the recording state, but there is nothing
                // to record yet. The main loop expects it to be closed, so close it now.
                m_commandList->Close();
            }

            void DX12Renderer::InitSynchronization()
            {
                m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
                m_fenceValue = 1;

                // Create an event handle to use for frame synchronization.
                m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                if (m_fenceEvent == nullptr)
                {
                    HRESULT_FROM_WIN32(GetLastError());
                }
            }

            void DX12Renderer::WaitForPreviousFrame()
            {
                // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
                // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
                // sample illustrates how to use fences for efficient resource usage and to
                // maximize GPU utilization.

                // Signal and increment the fence value.
                const UINT64 fence = m_fenceValue;
                m_commandQueue->Signal(m_fence.Get(), fence);
                m_fenceValue++;

                // Wait until the previous frame is finished.
                if (m_fence->GetCompletedValue() < fence)
                {
                    m_fence->SetEventOnCompletion(fence, m_fenceEvent);
                    WaitForSingleObject(m_fenceEvent, INFINITE);
                }

                m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
            }

            void DX12Renderer::RecordCmdList()
            {
                // Command list allocators can only be reset when the associated 
                // command lists have finished execution on the GPU; apps should use 
                // fences to determine GPU execution progress.
                m_commandAllocator->Reset();

                // However, when ExecuteCommandList() is called on a particular command 
                // list, that command list can then be reset at any time and must be before 
                // re-recording.
                m_commandList->Reset(m_commandAllocator.Get(),nullptr);

                // Indicate that the back buffer will be used as a render target.
                m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

                // Record commands.
                const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
                m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

                // Indicate that the back buffer will now be used to present.
                m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

                m_commandList->Close();
            }

            void DX12Renderer::Render()
            {
                // Record all the commands we need to render the scene into the command list.
                RecordCmdList();

                // Execute the command list.
                ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
                m_commandQueue->ExecuteCommandLists(1, ppCommandLists);

                // Present the frame.
                m_swapChain->Present(1, 0);

                WaitForPreviousFrame();
            }

            void DX12Renderer::InitCmdAllocator()
            {
                m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));

            }

            void DX12Renderer::GetHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter)
            {
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

            DX12Renderer::~DX12Renderer()
            {
                WaitForPreviousFrame();

                CloseHandle(m_fenceEvent);

            }
        }
    }
}