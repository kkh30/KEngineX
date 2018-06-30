#pragma once
#include "Factory.h"
#include "IRenderer.h"
#include <VulkanRenderer/VulkanRenderer.h>
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
                case ke::renderer::OGL:
                case ke::renderer::GLES:
                    _ASSERT(0);
                    return nullptr;
                    break;
                case ke::renderer::Vulkan:
                    return std::make_unique<VulkanRenderer>();
                    break;
                default:
                    return nullptr;
                    break;
                }
            }

        private:
            RendererFactory()
            {

            };

        };
        
    }
}