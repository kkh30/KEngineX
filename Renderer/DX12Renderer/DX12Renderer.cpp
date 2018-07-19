#include "DX12Renderer.h"



namespace ke
{
    namespace renderer
    {
        namespace dx12renderer
        {


            DX12Renderer::DX12Renderer() :
                m_frameIndex(0),
                m_rtvDescriptorSize(0),
                m_fenceEvent(nullptr),
                m_fenceValue(),
                m_viewport(0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)),
                m_scissorRect(0, 0, static_cast<LONG>(WINDOW_WIDTH), static_cast<LONG>(WINDOW_HEIGHT)),
                m_vertex_buffer(nullptr)
            {
            }
            
            void DX12Renderer::Init()
            {
                InitAPI();

                InitDescHeap();

                InitFramebuffer();

                InitCmdList();

                InitSynchronization();

                LoadScene();

                DX12API::GetAPI().RenderWindow();

            }

            void DX12Renderer::Update()
            {
            }

            void DX12Renderer::ShutDown()
            {

            }

            FrameGraph * DX12Renderer::GetFrameGraph()
            {
                return nullptr;
            }

            void DX12Renderer::InitAPI()
            {
                auto& api = DX12API::GetAPI();
                api.SetRenderFunc(std::bind(&DX12Renderer::Render, this));
                m_device = api.GetDevice();
                m_swapChain = api.GetSwapChain();
                m_commandQueue = api.GetCmdQueue();
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
                        IID_PPV_ARGS(&m_uploadBuffer));

                    // Copy the triangle data to the vertex buffer.
                    UINT8* pVertexDataBegin;
                    CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
                    m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
                    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
                    m_uploadBuffer->Unmap(0, nullptr);

                    m_vertex_buffer = new VertexBuffer(vertexBufferSize);
                   
                    m_commandList[0]->Reset(m_commandAllocator[0].Get(),nullptr);
                    D3D12_RESOURCE_BARRIER barrier = {};
                    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    barrier.Transition.pResource = m_vertex_buffer->GetResource();
                    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                    barrier.Transition.Subresource = 0;
                    m_commandList[0]->CopyBufferRegion(m_vertex_buffer->GetResource(), 0, m_uploadBuffer.Get(), 0, vertexBufferSize);
                    m_commandList[0]->ResourceBarrier(1, &barrier);
                    ID3D12CommandList* lists[1] = {m_commandList[0].Get()};
                    m_commandList[0]->Close();

                    m_commandQueue->ExecuteCommandLists(1, lists);
                    ComPtr<ID3D12Fence> l_fence;
                    HANDLE l_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

                    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&l_fence));

                    m_commandQueue->Signal(l_fence.Get(), 1);

                    while (l_fence->GetCompletedValue() != 1)
                    {
                        l_fence->SetEventOnCompletion(1, l_fenceEvent);
                        WaitForSingleObjectEx(l_fenceEvent, INFINITY, false);
                    }
                    m_commandAllocator[0]->Reset();
                    // Initialize the vertex buffer view.
                    m_vertexBufferView.BufferLocation = m_vertex_buffer->GetGpuVirtualAddress();
                    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
                    m_vertexBufferView.SizeInBytes = vertexBufferSize;
                }
            }

          
            

            DX12Renderer::~DX12Renderer()
            {
                WaitForPreviousFrame();
                delete m_vertex_buffer;
                CloseHandle(m_fenceEvent);

            }
        }
    }
}