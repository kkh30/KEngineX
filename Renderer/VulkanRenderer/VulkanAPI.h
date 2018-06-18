#pragma once
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <iostream>
#include <vector>
#include <assert.h>
#include <IAPI.h>
#include <stdint.h>


/** Macro to get a procedure address based on a Vulkan instance. */
#define GET_INSTANCE_PROC_ADDR(instance, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk"#name));

/** Macro to get a procedure address based on a Vulkan device. */
#define GET_DEVICE_PROC_ADDR(device, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk"#name));

namespace ke {

	namespace renderer {

		class VulkanAPI : public IAPI
		{
		public:
			~VulkanAPI();
			static VulkanAPI& GetVulkanAPI() {
				static VulkanAPI l_api;
				return l_api;
			};

		private:
			VulkanAPI();
			VkInstance m_instance = VK_NULL_HANDLE;
			VkDebugReportCallbackEXT m_debug_callback;
			// Inherited via IAPI
			virtual void Init(bool enable_validation = false) override;
			VkAllocationCallbacks* gVulkanAllocator = nullptr;
			PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = nullptr;
			PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = nullptr;
			PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
			PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
			PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
			PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
			PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
			PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;
			PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
			PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = nullptr;
			PFN_vkQueuePresentKHR vkQueuePresentKHR = nullptr;

		};
		
	}

	
}