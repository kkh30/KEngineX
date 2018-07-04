#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>
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

            private:


                // Inherited via IRenderer
                virtual void Init() override;

                virtual void Update() override;

                virtual void ShutDown() override;

                virtual FrameGraph * GetFrameGraph() override;

            };
        }
    }
}