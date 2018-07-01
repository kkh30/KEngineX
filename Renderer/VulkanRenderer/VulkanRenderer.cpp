#include "VulkanRenderer.h"
#include <EngineConstant.h>

namespace ke 
{
	namespace renderer 
    {
        namespace vkrenderer
        {
            VulkanRenderer::VulkanRenderer():m_frame_graph(nullptr)
            {
            }

            void VulkanRenderer::Render()
            {
            }

            FrameGraph * VulkanRenderer::GetFrameGraph()
            {
                return reinterpret_cast<FrameGraph*>(m_frame_graph);
            }

            void VulkanRenderer::Init()
            {
                InitAPI();

                InitWindow();

                InitSwapChain();
            }
            void VulkanRenderer::Update()
            {
                m_window_ptr->run();
            }
            void VulkanRenderer::ShutDown()
            {
                delete m_frame_graph;
                delete m_rendering_surface_ptr.release();
                delete m_swapchain_ptr.release();
                delete m_window_ptr.release();
            }
            void VulkanRenderer::InitAPI()
            {
                m_api = VulkanAPI::GetVulkanAPIPtr();
            }
            void VulkanRenderer::InitWindow()
            {
#ifdef ENABLE_OFFSCREEN_RENDERING
                const Anvil::WindowPlatform platform = Anvil::WINDOW_PLATFORM_DUMMY_WITH_PNG_SNAPSHOTS;
#else
#ifdef _WIN32
                const Anvil::WindowPlatform platform = Anvil::WINDOW_PLATFORM_SYSTEM;
#else
                const Anvil::WindowPlatform platform = Anvil::WINDOW_PLATFORM_XCB;
#endif
#endif

                /* Create a window */
                m_window_ptr = Anvil::WindowFactory::create_window(platform,
                    "KEngineX",
                    WINDOW_WIDTH,
                    WINDOW_HEIGHT,
                    true, /* in_closable */
                    std::bind(&VulkanRenderer::Render, this)
                );

            }
            void VulkanRenderer::InitSwapChain()
            {

                Anvil::SGPUDevice* device_ptr(m_api->m_device_ptr.get());

                m_rendering_surface_ptr = Anvil::RenderingSurface::create(m_api->m_instance_ptr.get(),
                    m_api->m_device_ptr.get(),
                    m_window_ptr.get());

                m_rendering_surface_ptr->set_name("Main rendering surface");


                m_swapchain_ptr = device_ptr->create_swapchain(m_rendering_surface_ptr.get(),
                    m_window_ptr.get(),
                    VK_FORMAT_B8G8R8A8_UNORM,
                    VK_PRESENT_MODE_FIFO_KHR,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    m_n_swapchain_images);

                m_swapchain_ptr->set_name("Main swapchain");

                /* Cache the queue we are going to use for presentation */
                const std::vector<uint32_t>* present_queue_fams_ptr = nullptr;

                if (!m_rendering_surface_ptr->get_queue_families_with_present_support(&present_queue_fams_ptr))
                {
                    anvil_assert_fail();
                }

                m_present_queue_ptr = device_ptr->get_queue_for_queue_family_index(present_queue_fams_ptr->at(0),
                    0); /* in_n_queue */

            }

            void VulkanRenderer::InitRenderPass()
            {
            }

            void VulkanRenderer::InitFramebuffers()
            {
                m_frame_graph = new VulkanFrameGraph;
            }

            void VulkanRenderer::InitPipelineState()
            {
            }

            void VulkanRenderer::InitFrameGraph()
            {
            }

            VulkanRenderer::~VulkanRenderer()
            {
                ShutDown();
            }
        }
	}
}