#pragma once
#include "IRenderer.h"
#include "VulkanAPI.h"
#include <EngineConstant.h>

namespace ke {
	namespace renderer {
		class VulkanRenderer : public IRenderer
		{
		public:
			VulkanRenderer();
			~VulkanRenderer();
			// Inherited via IModule
			virtual void Init() override;

			virtual void Update() override;

			virtual void ShutDown() override;
            
		private:
            Anvil::WindowUniquePtr           m_window_ptr;
            
            Anvil::RenderingSurfaceUniquePtr m_rendering_surface_ptr;

            Anvil::SwapchainUniquePtr        m_swapchain_ptr;
            
            Anvil::Queue*                    m_present_queue_ptr;

            const uint32_t m_n_swapchain_images = 3;

            VulkanAPI* m_api;
			
            void InitAPI();

            void InitWindow();

            void InitSwapChain();

            void Render();
		};

		
	}
}