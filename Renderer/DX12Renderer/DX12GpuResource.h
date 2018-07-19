//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once
#include <d3d12.h>
#include <Windows.h>
#include <wrl.h>
#include "d3dx12.h"

namespace ke
{
    namespace renderer
    {
        namespace dx12renderer
        {
            class GpuResource
            {

            public:
                GpuResource() :
                    m_GpuVirtualAddress(0),
                    //m_UserAllocatedMemory(nullptr),
                    m_UsageState(D3D12_RESOURCE_STATE_COMMON)//,
                                                             //m_TransitioningState((D3D12_RESOURCE_STATES)-1)
                {
                }

                GpuResource(D3D12_RESOURCE_STATES p_usage) :
                    m_GpuVirtualAddress(0),
                    //m_UserAllocatedMemory(nullptr),
                    m_UsageState(p_usage)//,
                                              //m_TransitioningState((D3D12_RESOURCE_STATES)-1)
                {
                }

                virtual ~GpuResource()                           
                {
                    //Destroy();
                }

                virtual void Destroy()
                {
                    m_pResource = nullptr;
                    m_GpuVirtualAddress = 0;
                    //if (m_UserAllocatedMemory != nullptr)
                    //{
                    //    VirtualFree(m_UserAllocatedMemory, 0, MEM_RELEASE);
                    //    m_UserAllocatedMemory = nullptr;
                    //}
                }

                ID3D12Resource* operator->() { return m_pResource.Get(); }
                const ID3D12Resource* operator->() const { return m_pResource.Get(); }

                ID3D12Resource* GetResource() { return m_pResource.Get(); }
                const ID3D12Resource* GetResource() const { return m_pResource.Get(); }

                D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

            protected:

                Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
                D3D12_RESOURCE_STATES m_UsageState;
                //D3D12_RESOURCE_STATES m_TransitioningState;
                D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;

                // When using VirtualAlloc() to allocate memory directly, record the allocation here so that it can be freed.  The
                // GpuVirtualAddress may be offset from the true allocation start.
                //void* m_UserAllocatedMemory;
            };

        }
    }
}

