#pragma once
#include "DX12API.h"
#include <IRenderer.h>
#include <FrameGraph.h>

namespace ke
{
    namespace renderer
    {
        namespace dx12renderer
        {

            class DX12Renderer final: public IRenderer
            {
            public:
                DX12Renderer();
                ~DX12Renderer();

            public:
                // Inherited via IRenderer
                virtual void Init() override;

                virtual void Update() override;

                virtual void ShutDown() override;

                virtual FrameGraph * GetFrameGraph() override;
            private:
                void InitAPI();

                void InitSwapChain();

                void InitDescHeap();

                void InitFramebuffer();

                void InitCmdList();

                void InitSynchronization();

                void WaitForPreviousFrame();

                void RecordCmdList();

                void Render();

                void LoadScene();

                void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);


                ComPtr<ID3D12CommandAllocator> m_commandAllocator[SWAP_CHAIN_COUNT];
                
                ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

                ComPtr<ID3D12Resource> m_renderTargets[SWAP_CHAIN_COUNT];

                ComPtr<ID3D12GraphicsCommandList> m_commandList[SWAP_CHAIN_COUNT];

                ComPtr<ID3D12RootSignature> m_rootSignature;

                ComPtr<ID3D12PipelineState> m_pipelineState;

                ComPtr<ID3D12Resource> m_vertexBuffer;

                D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

                ComPtr<ID3D12Device> m_device;

                ComPtr<ID3D12CommandQueue> m_commandQueue;

                ComPtr<IDXGISwapChain3> m_swapChain;

                CD3DX12_VIEWPORT m_viewport;

                CD3DX12_RECT m_scissorRect;

                // Synchronization objects.
                HANDLE m_fenceEvent;
                ComPtr<ID3D12Fence> m_fences[SWAP_CHAIN_COUNT];
                UINT64 m_fenceValue[SWAP_CHAIN_COUNT];
                uint64_t m_current_value = 0;
                uint32_t m_frameIndex = 0;

                uint32_t m_rtvDescriptorSize = 0;

            };
        }
    }
}