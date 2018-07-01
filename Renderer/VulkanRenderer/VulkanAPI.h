#pragma once
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32

#include <INoCopy.h>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <iostream>
#include <vector>
#include <assert.h>
#include <IAPI.h>
#include <stdint.h>
#include <config.h>
#include "misc/buffer_create_info.h"
#include "misc/framebuffer_create_info.h"
#include "misc/glsl_to_spirv.h"
#include "misc/graphics_pipeline_create_info.h"
#include "misc/image_create_info.h"
#include "misc/image_view_create_info.h"
#include "misc/io.h"
#include "misc/memory_allocator.h"
#include "misc/object_tracker.h"
#include "misc/render_pass_create_info.h"
#include "misc/semaphore_create_info.h"
#include "misc/swapchain_create_info.h"
#include "misc/time.h"
#include "misc/window_factory.h"
#include "wrappers/buffer.h"
#include "wrappers/command_buffer.h"
#include "wrappers/command_pool.h"
#include "wrappers/descriptor_set_group.h"
#include "wrappers/descriptor_set_layout.h"
#include "wrappers/device.h"
#include "wrappers/event.h"
#include "wrappers/graphics_pipeline_manager.h"
#include "wrappers/framebuffer.h"
#include "wrappers/image.h"
#include "wrappers/image_view.h"
#include "wrappers/instance.h"
#include "wrappers/physical_device.h"
#include "wrappers/query_pool.h"
#include "wrappers/rendering_surface.h"
#include "wrappers/render_pass.h"
#include "wrappers/semaphore.h"
#include "wrappers/shader_module.h"
#include "wrappers/swapchain.h"


/** Macro to get a procedure address based on a Vulkan instance. */
#define GET_INSTANCE_PROC_ADDR(instance, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetInstanceProcAddr(instance, "vk"#name));

/** Macro to get a procedure address based on a Vulkan device. */
#define GET_DEVICE_PROC_ADDR(device, name) \
	vk##name = reinterpret_cast<PFN_vk##name>(vkGetDeviceProcAddr(device, "vk"#name));

namespace ke {

	namespace renderer {

        namespace vkrenderer 
        {
            class VulkanAPI final : public IAPI
            {
            public:

                static VulkanAPI& GetVulkanAPI() {
                    return *GetVulkanAPIPtr();
                }

                static VulkanAPI* GetVulkanAPIPtr()
                {
                    static VulkanAPI l_api;
                    return &l_api;
                }

                ~VulkanAPI();

            private:

                void init_instance_device();
                
                VulkanAPI();

            private:
                
                // Inherited via IAPI
                virtual void Init(bool enable_validation = false) override;

            public:
                // Anvil
                Anvil::InstanceUniquePtr         m_instance_ptr;
                Anvil::SGPUDeviceUniquePtr       m_device_ptr;
                const Anvil::PhysicalDevice*     m_physical_device_ptr;
                VmaAllocator*    m_vma_allocator;
                VkDebugReportCallbackEXT m_debug_callback;

            private:
                // Vulkan
                VkAllocationCallbacks * gVulkanAllocator = nullptr;
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
                bool m_enable_validation = false;
                VkBool32 debugMsgCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
                    size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);
            };

        }

		
	}

	
}