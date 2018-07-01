#pragma once
#include <RenderPass.h>

namespace ke
{
    namespace renderer
    {
        class FrameGraph : public INoCopy
        {
        public:
            FrameGraph() {
            
            }
            virtual ~FrameGraph() {
            
            }

            __forceinline void AddRenderPass(UniquePtr<RenderPass>&& p_pass)
            {
                m_render_pass.push_back(std::forward<UniquePtr<RenderPass>>(p_pass));
            }

            virtual void BuildFrameGraph()
            {
            }

        private:
            std::vector<UniquePtr<RenderPass>> m_render_pass;

        };

       
    }
}
