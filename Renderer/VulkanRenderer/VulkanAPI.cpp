#include "VulkanAPI.h"


namespace ke {

	namespace renderer {
		
		VulkanAPI::VulkanAPI()
		{
			Init(true);
		}

		
		void VulkanAPI::Init(bool enable_validation)
		{
			m_enable_validation = enable_validation;
			init_vulkan();
			init_window();
			init_swapchain();
		}

		VulkanAPI::~VulkanAPI()
		{
		}

		
		void VulkanAPI::init_vulkan()
		{
			/* Create a Vulkan instance */
			if (m_enable_validation) {
				m_instance_ptr = Anvil::Instance::create("KEngineX",  /* in_app_name    */
					"KEngineX",  /* in_engine_name */
					std::bind(&VulkanAPI::debugMsgCallback,
						this,
						std::placeholders::_1,
						std::placeholders::_2,
						std::placeholders::_3,
						std::placeholders::_4,
						std::placeholders::_5,
						std::placeholders::_6,
						std::placeholders::_7,
						std::placeholders::_8
						),
					false); /* in_mt_safe */
			}
			else {
				m_instance_ptr = Anvil::Instance::create("KEngineX",  /* in_app_name    */
					"KEngineX",  /* in_engine_name */
					Anvil::DebugCallbackFunction(),
					false); /* in_mt_safe */
			}

			m_physical_device_ptr = m_instance_ptr->get_physical_device(0);

			/* Create a Vulkan device */
			m_device_ptr = Anvil::SGPUDevice::create(m_physical_device_ptr,
				true,                       /* in_enable_shader_module_cache           */
				Anvil::DeviceExtensionConfiguration(),
				std::vector<std::string>(), /* in_layers                               */
				false,                      /* in_transient_command_buffer_allocs_only */
				false);                     /* in_support_resettable_command_buffers   */
		}
		void VulkanAPI::init_window()
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
				1920,
				1080,
				true, /* in_closable */
				[]() {}
			);
		}
		void VulkanAPI::init_swapchain()
		{
			Anvil::SGPUDevice* device_ptr(m_device_ptr.get());

			m_rendering_surface_ptr = Anvil::RenderingSurface::create(m_instance_ptr.get(),
				m_device_ptr.get(),
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



		VkBool32 VulkanAPI::debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
			size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData)
		{

			// Determine prefix
			if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
				std::cout << "ERROR";

			if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
				std::cout << "WARNING";

			if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
				std::cout << "PERFORMANCE";

			if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
				std::cout << "INFO";

			if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
				std::cout << "DEBUG";

			std::cout << ": [" << pLayerPrefix << "] Code " << msgCode << ": " << pMsg << std::endl;

			// Don't abort calls that caused a validation message
			return VK_FALSE;
		}
		

	}
}




