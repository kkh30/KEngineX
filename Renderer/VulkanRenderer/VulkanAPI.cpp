#include "VulkanAPI.h"
VkBool32 debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
	size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

namespace ke {

	namespace renderer {

		VulkanAPI::VulkanAPI()
		{
			Init(true);
		}

		
		void VulkanAPI::Init(bool enable_validation)
		{
			// Create instance
			VkApplicationInfo appInfo;
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pNext = nullptr;
			appInfo.pApplicationName = "KEngineX";
			appInfo.applicationVersion = 1;
			appInfo.pEngineName = "KEngineX";
			appInfo.apiVersion = VK_API_VERSION_1_0;

			std::vector<const char*> layers = {};
			std::vector<const char*> extensions = { nullptr,nullptr };
			uint32_t numLayers = 0;

			if (enable_validation) {
				layers.push_back("VK_LAYER_LUNARG_standard_validation");
				extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				numLayers = sizeof(layers) / sizeof(layers[0]);
			}

			extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;

#ifdef _WIN32
			extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif

			VkInstanceCreateInfo instanceInfo;
			instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instanceInfo.pNext = nullptr;
			instanceInfo.flags = 0;
			instanceInfo.pApplicationInfo = &appInfo;
			instanceInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
			instanceInfo.ppEnabledLayerNames = layers.data();
			instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			instanceInfo.ppEnabledExtensionNames = extensions.data();

			VkResult result = vkCreateInstance(&instanceInfo, gVulkanAllocator, &m_instance);
			assert(result == VK_SUCCESS);

			// Set up debugging
			if (enable_validation) {
				VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
					VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

				GET_INSTANCE_PROC_ADDR(m_instance, CreateDebugReportCallbackEXT);
				GET_INSTANCE_PROC_ADDR(m_instance, DestroyDebugReportCallbackEXT);

				VkDebugReportCallbackCreateInfoEXT debugInfo;
				debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
				debugInfo.pNext = nullptr;
				debugInfo.flags = 0;
				debugInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugMsgCallback;
				debugInfo.flags = debugFlags;

				result = vkCreateDebugReportCallbackEXT(m_instance, &debugInfo, nullptr, &m_debug_callback);
				assert(result == VK_SUCCESS);
			}

			// Enumerate all devices
			uint32_t mNumDevices = 0;
			result = vkEnumeratePhysicalDevices(m_instance, &mNumDevices, nullptr);
			assert(result == VK_SUCCESS);

			std::vector<VkPhysicalDevice> physicalDevices(mNumDevices);
			result = vkEnumeratePhysicalDevices(m_instance, &mNumDevices, physicalDevices.data());
			assert(result == VK_SUCCESS);

			//mDevices.resize(mNumDevices);
			//for (uint32_t i = 0; i < mNumDevices; i++)
			//	mDevices[i] = bs_shared_ptr_new<VulkanDevice>(physicalDevices[i], i);
		}

		VulkanAPI::~VulkanAPI()
		{
			vkDestroyInstance(m_instance, nullptr);
		}

		

		

	}
}


static VkBool32 debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
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

