
#pragma once

#include	<vector>

#define DEFAULT_FENCE_TIMEOUT 100000000000

struct QueueFamilyIndices 
{
	enum
	{
		noValue = UINT32_MAX
	};

	uint32_t graphicsFamily = noValue;
	uint32_t presentFamily  = noValue;
	uint32_t computeFamily  = noValue;

	bool isComplete() const
	{
		return (graphicsFamily != noValue) && (presentFamily != noValue);
	}
};

class	Device
{
	VkInstance						instance            = VK_NULL_HANDLE;	// vulkan instance for app
	VkDebugUtilsMessengerEXT		debugMessenger      = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT 		msgCallback         = VK_NULL_HANDLE;
	VkPhysicalDevice				physicalDevice      = VK_NULL_HANDLE;
	VkDevice						device              = VK_NULL_HANDLE;
	VkQueue							graphicsQueue       = VK_NULL_HANDLE;
	VkQueue							presentQueue        = VK_NULL_HANDLE;
	VkQueue							computeQueue        = VK_NULL_HANDLE;
	VkCommandPool					commandPool         = VK_NULL_HANDLE;
	uint32_t						graphicsFamilyIndex = UINT32_MAX;
	uint32_t						presentFamilyIndex  = UINT32_MAX;
	uint32_t						computeFamilyIndex  = UINT32_MAX;

	friend class VulkanWindow;
	
public:
	Device  () {}
	~Device () {}

	VkDevice	getDevice () const
	{
		return device;
	}
	
	VkInstance	getInstance () const
	{
		return instance;
	}
	
	VkPhysicalDevice	getPhysicalDevice () const
	{
		return physicalDevice;
	}
	
	VkQueue	getGraphicsQueue () const
	{
		return graphicsQueue;
	}
	
	VkQueue	getPresentQueue () const
	{
		return presentQueue;
	}
	
	VkQueue	getComputeQueue () const
	{
		return computeQueue;
	}
	
	uint32_t	getGraphicsFamilyIndex () const
	{
		return graphicsFamilyIndex;
	}
	
	uint32_t	getPresentFamilyIndex () const
	{
		return presentFamilyIndex;
	}
	
	uint32_t	getComputeFamilyIndex () const
	{
		return computeFamilyIndex;
	}
	
	VkCommandPool	getCommandPool () const
	{
		return commandPool;
	}
/*
	void	pickPhysicalDevice  ();
	void	createLogicalDevice ();  	// needs surface !!!
	void	createCommandPool   ();
*/

	static QueueFamilyIndices findQueueFamilies ( VkPhysicalDevice device, VkSurfaceKHR surface = nullptr )
	{
		QueueFamilyIndices	indices;
		uint32_t			queueFamilyCount = 0;
		int 				i                = 0;

		vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, nullptr );

		std::vector<VkQueueFamilyProperties> queueFamilies ( queueFamilyCount );

		vkGetPhysicalDeviceQueueFamilyProperties ( device, &queueFamilyCount, queueFamilies.data () );

		for ( const auto& queueFamily : queueFamilies )
		{
			if ( indices.graphicsFamily == QueueFamilyIndices::noValue && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
				indices.graphicsFamily = i;

			if ( indices.computeFamily == QueueFamilyIndices::noValue && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT )
				indices.computeFamily = i;

			VkBool32 presentSupport = false;

			if ( surface != nullptr )
				vkGetPhysicalDeviceSurfaceSupportKHR ( device, i, surface, &presentSupport );

			if ( presentSupport  )
				indices.presentFamily = i;

			if ( indices.isComplete () )
				return indices;

			i++;
		}

		return indices;
	}
};