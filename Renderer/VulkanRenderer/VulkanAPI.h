#pragma once
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <iostream>
#include <vector>
#include <assert.h>

#define USE_VALIDATION_LAYERS

/** Macro to get a procedure address based on a Vulkan instance. */
#define GET_INSTANCE_PROC_ADDR(instance, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk"#name));

/** Macro to get a procedure address based on a Vulkan device. */
#define GET_DEVICE_PROC_ADDR(device, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk"#name));

namespace ke {
	class VulkanAPI
	{
	public:
		VulkanAPI();
		~VulkanAPI();

	private:
		VkInstance m_instance = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT m_debug_callback;


	};
	extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
	extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
	extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

	extern PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
	extern PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
	extern PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
	extern PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
	extern PFN_vkQueuePresentKHR vkQueuePresentKHR;
}