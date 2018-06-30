#include "VulkanAPI.h"

namespace ke
{

    namespace renderer
    {

        VulkanAPI::VulkanAPI()
        {
            Init(true);
        }


        void VulkanAPI::Init(bool enable_validation)
        {
            m_enable_validation = enable_validation;
            init_instance_device();
        }

        VulkanAPI::~VulkanAPI()
        {
        }


        void VulkanAPI::init_instance_device()
        {
            /* Create a Vulkan instance */
            if (m_enable_validation)
            {
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
            else
            {
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




