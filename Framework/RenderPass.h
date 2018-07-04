#pragma once
#include <FrameResource.h>
#include <vector>
#include <EngineConstant.h>
namespace ke
{
    namespace renderer
    {
        class RenderPass
        {
        public:
            RenderPass() :m_inputs({}), m_target(nullptr)
            {
            
            }
            virtual ~RenderPass()
            {

            }

            __forceinline void AddInput(SharedPtr<FrameResource> p_resource)
            {
                m_inputs.push_back(p_resource);
            }
            __forceinline void SetTarget(SharedPtr<FrameResource> p_resource)
            {
                m_target = p_resource;
            }

        private:
            std::vector<SharedPtr<FrameResource>> m_inputs;
            SharedPtr<FrameResource> m_target;
        };
    }
}