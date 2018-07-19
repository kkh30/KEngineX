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
            
            class VertexBuffer final: public GpuResource, public INoCopy
            {
            public:
                VertexBuffer(uint64_t p_size);
                ~VertexBuffer();

            private:
                //Vertex Buffer size in byte
                uint64_t m_size;

            };

           
        }
    }
}