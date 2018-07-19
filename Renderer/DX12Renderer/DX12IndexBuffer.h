#pragma once
#include "DX12GpuResource.h"
#include <INoCopy.h>
#include <stdint.h>
#include "DX12API.h"

namespace ke
{
    namespace renderer
    {
        namespace dx12renderer
        {

            class IndexBuffer final : public GpuResource, public INoCopy
            {
            public:
                IndexBuffer(uint64_t p_size);
                ~IndexBuffer();

            private:
                //Vertex Buffer size in byte
                uint64_t m_size;

            };

            IndexBuffer::IndexBuffer(uint64_t p_size) :
                GpuResource(D3D12_RESOURCE_STATE_INDEX_BUFFER),
                m_size(p_size)
            {
                auto& api = DX12API::GetAPI();

                api.GetDevice()->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(m_size),
                    D3D12_RESOURCE_STATE_INDEX_BUFFER,
                    nullptr, IID_PPV_ARGS(&m_pResource));


            }

            IndexBuffer::~IndexBuffer()
            {
            }
        }
    }
}