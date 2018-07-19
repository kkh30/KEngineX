#include "DX12VertexBuffer.h"


namespace ke
{
    namespace renderer
    {
        namespace dx12renderer
        {
            VertexBuffer::VertexBuffer(uint64_t p_size) :
                GpuResource(D3D12_RESOURCE_STATE_COPY_DEST),
                m_size(p_size)
            {
                auto& api = DX12API::GetAPI();

                api.GetDevice()->CreateCommittedResource(
                    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                    D3D12_HEAP_FLAG_NONE,
                    &CD3DX12_RESOURCE_DESC::Buffer(m_size),
                    m_UsageState,
                    nullptr, IID_PPV_ARGS(&m_pResource));

                m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
            }

            VertexBuffer::~VertexBuffer()
            {
                m_pResource = nullptr;
            }
        }
    }
}