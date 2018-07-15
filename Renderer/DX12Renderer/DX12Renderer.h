#pragma once
#include "DX12Renderer/Win32Application.h"
#include "d3dx12.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <IRenderer.h>
#include <FrameGraph.h>
#include <EngineConstant.h>

namespace ke
{
    namespace renderer
    {
        namespace dx12renderer
        {
            using Microsoft::WRL::ComPtr;

            class DX12Renderer final: public IRenderer,public Win32Application
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
                void InitDevice();

                void InitSwapChain();

                void InitDescHeap();

                void InitFramebuffer();

                void InitCmdList();

                void InitSynchronization();

                void WaitForPreviousFrame();

                void RecordCmdList();

                void Render();


                bool m_useWarpDevice = false;

                void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
                
                ComPtr<ID3D12CommandQueue> m_commandQueue;

                ComPtr<IDXGISwapChain3> m_swapChain;
                
                ComPtr<ID3D12Device> m_device;

                ComPtr<ID3D12CommandAllocator> m_commandAllocator[SWAP_CHAIN_COUNT];
                
                ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

                ComPtr<ID3D12Resource> m_renderTargets[SWAP_CHAIN_COUNT];

                ComPtr<ID3D12GraphicsCommandList> m_commandList[SWAP_CHAIN_COUNT];

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