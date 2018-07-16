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
                m_fenceEvent(nullptr),
                m_fenceValue(),
                m_viewport(0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)),
                m_scissorRect(0, 0, static_cast<LONG>(WINDOW_WIDTH), static_cast<LONG>(WINDOW_HEIGHT))
            {
            }
            
            void DX12Renderer::Init()
            {
                InitDevice();

                InitDescHeap();

                InitFramebuffer();

                InitCmdList();

                InitSynchronization();

                LoadScene();

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

                for (auto i = 0; i < SWAP_CHAIN_COUNT; ++i)
                {
                    m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i]));
                    // Create the command list.
                    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[i].Get(), nullptr, IID_PPV_ARGS(&m_commandList[i]));

                    // Command lists are created in the recording state, but there is nothing
                    // to record yet. The main loop expects it to be closed, so close it now.
                    m_commandList[i]->Close();
                }
                
            }

            void DX12Renderer::InitSynchronization()
            {
                m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fences[0]));
                m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fences[1]));
                m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fences[2]));

                // Create an event handle to use for frame synchronization.
                m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                if (m_fenceEvent == nullptr)
                {
                    HRESULT_FROM_WIN32(GetLastError());
                }
            }

            void DX12Renderer::WaitForPreviousFrame()
            {
                //shcedule a fence in command queue,based on the frame that
                //rendering is working on.
                m_commandQueue->Signal(m_fences[m_frameIndex].Get(), m_fenceValue[m_frameIndex]);
                m_fenceValue[m_frameIndex] = m_current_value++;
                m_frameIndex = (m_frameIndex + 1) % 3;

                const auto current_swapchain_index = m_swapChain->GetCurrentBackBufferIndex();
                if (m_fences[current_swapchain_index]->GetCompletedValue() < m_fenceValue[current_swapchain_index]) 
                {
                    m_fences[current_swapchain_index]->SetEventOnCompletion(m_fenceValue[current_swapchain_index], m_fenceEvent);
                    WaitForSingleObjectEx(m_fenceEvent, INFINITY, false);
                }

            }

            void DX12Renderer::RecordCmdList()
            {
                // Command list allocators can only be reset when the associated 
                // command lists have finished execution on the GPU; apps should use 
                // fences to determine GPU execution progress.
                m_commandAllocator[m_frameIndex]->Reset();

                // However, when ExecuteCommandList() is called on a particular command 
                // list, that command list can then be reset at any time and must be before 
                // re-recording.
                m_commandList[m_frameIndex]->Reset(m_commandAllocator[m_frameIndex].Get(), m_pipelineState.Get());

                // Set necessary state.
                m_commandList[m_frameIndex]->SetGraphicsRootSignature(m_rootSignature.Get());
                m_commandList[m_frameIndex]->RSSetViewports(1, &m_viewport);
                m_commandList[m_frameIndex]->RSSetScissorRects(1, &m_scissorRect);

                // Indicate that the back buffer will be used as a render target.
                m_commandList[m_frameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
                m_commandList[m_frameIndex]->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

                // Record commands.
                const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
                m_commandList[m_frameIndex]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
                m_commandList[m_frameIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                m_commandList[m_frameIndex]->IASetVertexBuffers(0, 1, &m_vertexBufferView);
                m_commandList[m_frameIndex]->DrawInstanced(3, 1, 0, 0);
                // Indicate that the back buffer will now be used to present.
                m_commandList[m_frameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

                m_commandList[m_frameIndex]->Close();
            }

            void DX12Renderer::Render()
            {
                // Record all the commands we need to render the scene into the command list.
                RecordCmdList();

                // Execute the command list.
                ID3D12CommandList* ppCommandLists[] = { m_commandList[m_frameIndex].Get() };
                m_commandQueue->ExecuteCommandLists(1, ppCommandLists);

                // Present the frame.
                m_swapChain->Present(1, 0);

                WaitForPreviousFrame();
            }

            void DX12Renderer::LoadScene()
            {
                // Create an empty root signature.
                {
                    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
                    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

                    ComPtr<ID3DBlob> signature;
                    ComPtr<ID3DBlob> error;
                    D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
                    m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
                }

                // Create the pipeline state, which includes compiling and loading shaders.
                {
                    ComPtr<ID3DBlob> vertexShader;
                    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
                    // Enable better shader debugging with the graphics debugging tools.
                    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
                    UINT compileFlags = 0;
#endif

                    D3DCompileFromFile(L"D:\\Dev\\KEngine2018\\Build\\Game\\Debug\\shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
                    D3DCompileFromFile(L"D:\\Dev\\KEngine2018\\Build\\Game\\Debug\\shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

                    // Define the vertex input layout.
                    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
                    {
                        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
                    };

                    // Describe and create the graphics pipeline state object (PSO).
                    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
                    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
                    psoDesc.pRootSignature = m_rootSignature.Get();
                    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
                    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
                    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
                    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
                    psoDesc.DepthStencilState.DepthEnable = FALSE;
                    psoDesc.DepthStencilState.StencilEnable = FALSE;
                    psoDesc.SampleMask = UINT_MAX;
                    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                    psoDesc.NumRenderTargets = 1;
                    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
                    psoDesc.SampleDesc.Count = 1;
                    m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
                }


                // Create the vertex buffer.
                {
                    struct Vertex
                    {
                        float position[3];
                        float color[4];
                    };
                    // Define the geometry for a triangle.
                    Vertex triangleVertices[] =
                    {
                        { { 0.0f, 0.25f  , 0.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
                    { { 0.25f, -0.25f  , 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
                    { { -0.25f, -0.25f  , 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
                    };

                    const UINT vertexBufferSize = sizeof(triangleVertices);

                    // Note: using upload heaps to transfer static data like vert buffers is not 
                    // recommended. Every time the GPU needs it, the upload heap will be marshalled 
                    // over. Please read up on Default Heap usage. An upload heap is used here for 
                    // code simplicity and because there are very few verts to actually transfer.
                    m_device->CreateCommittedResource(
                        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                        D3D12_HEAP_FLAG_NONE,
                        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
                        D3D12_RESOURCE_STATE_GENERIC_READ,
                        nullptr,
                        IID_PPV_ARGS(&m_vertexBuffer));

                    // Copy the triangle data to the vertex buffer.
                    UINT8* pVertexDataBegin;
                    CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
                    m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
                    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
                    m_vertexBuffer->Unmap(0, nullptr);

                    // Initialize the vertex buffer view.
                    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
                    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
                    m_vertexBufferView.SizeInBytes = vertexBufferSize;
                }
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