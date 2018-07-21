#pragma once
#include "IAPI.h"
#include "d3dx12.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <windows.h>
#include <wrl.h>
#include <EngineConstant.h>
#include "Win32Application.h"
using Microsoft::WRL::ComPtr;

namespace ke
{
    namespace renderer
    {
        namespace dx12renderer
        {

            class DX12API final: public IAPI, public Win32Application
            {
            public:

                static DX12API& GetAPI() {
                    static DX12API l_api;
                    return l_api;
                }
                
                ~DX12API();

                __forceinline ComPtr<ID3D12Device> GetDevice() 
                {
                    return m_device;
                };

                __forceinline ComPtr<ID3D12CommandQueue> GetCmdQueue()
                {
                    return m_commandQueue;
                };

                __forceinline ComPtr<IDXGISwapChain3> GetSwapChain()
                {
                    return m_swapChain;
                };
                
                ComPtr<ID3D12GraphicsCommandList> CreateGraphicsCommandList();


                bool TransitResource(ID3D12Resource* p_resource, D3D12_RESOURCE_STATES, D3D12_RESOURCE_STATES);

                bool FlushCmdQueue(ComPtr<ID3D12GraphicsCommandList> p_list,bool release_cmd_list = true);

                bool WaitForCmdQueue();

            private:
                
                DX12API();

                // Inherited via IAPI
                virtual void Init(bool enable_validation = false) override;

                bool m_useWarpDevice = false;

                bool m_enable_debug = false;

                ComPtr<ID3D12Fence> m_fence;

                HANDLE m_fence_event;

                uint64_t m_current_fence_value;

                ComPtr<ID3D12Device> m_device;

                ComPtr<ID3D12CommandQueue> m_commandQueue;
                
                ComPtr<IDXGISwapChain3> m_swapChain;

                ComPtr<ID3D12CommandAllocator> m_cmd_allocator;

                __forceinline void BlockToWait();
            };
        }
    }
}