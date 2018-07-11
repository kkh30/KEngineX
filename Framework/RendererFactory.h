#pragma once
#include "Factory.h"
#include "IRenderer.h"
#include <VulkanRenderer/VulkanRenderer.h>
#include <DX12Renderer/DX12Renderer.h>
#include <EngineConstant.h>
namespace ke
{
    namespace renderer
    {
        class RendererFactory final: public Factory<IRenderer>
        {
        public:

            static RendererFactory& GetFactory() {
                static RendererFactory l_factory;
                return l_factory;
            }
            
            ~RendererFactory()
            {

            };

            UniquePtr<IRenderer> Create(RendererAPI&& p_api){

                switch (p_api)
                {
                case ke::renderer::DirextX:
                    m_current_renderer = std::make_unique<dx12renderer::DX12Renderer>();
                    return std::forward<UniquePtr<IRenderer>>(m_current_renderer);
                case ke::renderer::OGL:
                case ke::renderer::GLES:
                    _ASSERT(0);
                    return nullptr;
                case ke::renderer::Vulkan:
                    return std::make_unique<vkrenderer::VulkanRenderer>();
                default:
                    return nullptr;
                    break;
                }
            }

        private:
            RendererFactory()
            {
            };
            UniquePtr<IRenderer> m_current_renderer;
        };
        
    }
}