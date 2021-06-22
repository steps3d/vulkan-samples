#include	"VulkanWindow.h"
#include	"Buffer.h"
#include	"Texture.h"

const std::vector<const char*> validationLayers = 
{
    "VK_LAYER_KHRONOS_validation",
	"VK_LAYER_LUNARG_standard_validation" /*,
	"VK_LAYER_LUNARG_core_validation", 
	"VK_LAYER_LUNARG_threading",
	"VK_LAYER_LUNARG_mem_tracker",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_draw_state",
	"VK_LAYER_LUNARG_param_checker",
	"VK_LAYER_LUNARG_swapchain",
	"VK_LAYER_LUNARG_device_limits",
	"VK_LAYER_LUNARG_image",
	"VK_LAYER_GOOGLE_unique_objects",
	"VK_KHR_surface",
	"VK_EXT_debug_report"
*/
};

const std::vector<const char*> deviceExtensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

/*
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
*/

const bool enableValidationLayers = true;

	// GLFW callbacks
static void _key ( GLFWwindow * window, int key, int scanCode, int action, int mods )
{
	VulkanWindow * win = (VulkanWindow *) glfwGetWindowUserPointer ( window );
		
	if ( win != nullptr )
		win -> keyTyped ( key, scanCode, action, mods );
	else
	if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
		glfwSetWindowShouldClose ( window, GLFW_TRUE );
}

static void	_reshape ( GLFWwindow * window, int w, int h )
{
	VulkanWindow * win = (VulkanWindow *) glfwGetWindowUserPointer (window );
		
	if ( win != nullptr )
		win -> reshape ( w, h );
}
	
static void _mouse ( GLFWwindow* window, double xpos, double ypos )
{
	VulkanWindow * win = (VulkanWindow *) glfwGetWindowUserPointer (window );
		
	if ( win != nullptr )
		win -> mouseMotion ( xpos, ypos );
}

static void _mouseClick ( GLFWwindow* window, int button, int action, int mods )
{
	VulkanWindow * win = (VulkanWindow *) glfwGetWindowUserPointer (window );
		
	if ( win != nullptr )
		win -> mouseClick ( button, action, mods );
}

static void _mouseScroll ( GLFWwindow* window, double xoffset, double yoffset )
{
	VulkanWindow * win = (VulkanWindow *) glfwGetWindowUserPointer (window );
		
	if ( win != nullptr )
		win -> mouseWheel ( xoffset, yoffset );
}	

////////////////////// VulkanWindow methods ///////////////////////////////

void VulkanWindow::initWindow ( int w, int h, const std::string& t ) 
{
	if ( !glfwInit () )
		fatal () << "VulkanWindow: error initializing GLFW" << Log::endl;
		
	glfwWindowHint ( GLFW_CLIENT_API, GLFW_NO_API );
	//glfwWindowHint ( GLFW_RESIZABLE,  GLFW_FALSE  );

	width  = w;
	height = h;
	title  = t;
	window = glfwCreateWindow ( w, h, t.c_str (), nullptr, nullptr );

				// set callbacks
	glfwSetWindowUserPointer   ( window, this         );
	glfwSetKeyCallback         ( window, _key         );
	glfwSetWindowSizeCallback  ( window, _reshape     );
	glfwSetCursorPosCallback   ( window, _mouse       );	
	glfwSetMouseButtonCallback ( window, _mouseClick  );
	glfwSetScrollCallback      ( window, _mouseScroll );
}

void	VulkanWindow::initVulkan ()
{
	createInstance      ();
	setupDebugMessenger ();
		
	createSurface       ();
	pickPhysicalDevice  ();
	createLogicalDevice ();

	swapChain.createSwapChain ( device, surface, window, width, height );

	createCommandPool    ();
	createDepthTexture   ();

	swapChain.createSyncObjects ();
}

void	VulkanWindow::cleanup ()
{
	swapChain.cleanup  ();
	depthTexture.clean ();
	
	destroyCommandPool ();

	vkDestroyDevice    ( device.getDevice (), nullptr );

	if ( enableValidationLayers )
		destroyDebugUtilsMessengerEXT ( device.getInstance (), debugMessenger, nullptr );
		
	vkDestroySurfaceKHR ( device.getInstance (), surface, nullptr );
	vkDestroyInstance   ( device.getInstance (), nullptr );

	glfwDestroyWindow   ( window );
	glfwTerminate       ();
}

void	VulkanWindow::mainLoop () 
{
	while ( !glfwWindowShouldClose ( window ) )
	{
		glfwPollEvents ();
		drawFrame      ();
		updateFps      ();
		idle           ();
	}

	vkDeviceWaitIdle ( device.getDevice () );
}

void	VulkanWindow::recreateSwapChain ()
{
			// get new window size
	int width = 0, height = 0;
		
	while ( width == 0 || height == 0 )
	{
		glfwGetFramebufferSize ( window, &width, &height );
		glfwWaitEvents         ();
	}

			// wait till can do anything
	vkDeviceWaitIdle( device.getDevice () );

			// clean and recreate swap chain
	cleanupSwapChain ();

	swapChain.createSwapChain ( device, surface, window, width, height );

			// create depth texture
	createDepthTexture ();
	
			// recreate pipelines, renderpasses and command buffers
	createPipelines ();
}

void	VulkanWindow::createInstance () 
{
	if ( enableValidationLayers && !checkValidationLayerSupport () )
		fatal () << "VulkanWindow: validation layers requested, but not available!";

	uint32_t 		glfwExtensionCount = 0;
	auto			extensions         = getRequiredExtensions();
	VkApplicationInfo	appInfo        = {};

	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = appName.c_str ();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName        = engineName.c_str ();
	appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};

	createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo        = &appInfo;
	createInfo.enabledLayerCount       = 0;
	createInfo.enabledExtensionCount   = static_cast<uint32_t>( extensions.size () );
	createInfo.ppEnabledExtensionNames = extensions.data ();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	
	if ( enableValidationLayers ) 
	{
		createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
		
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if ( vkCreateInstance ( &createInfo, nullptr, &device.instance ) != VK_SUCCESS )
		fatal () << "VulkanWindow:failed to create instance!";
}

void	VulkanWindow::populateDebugMessengerCreateInfo ( VkDebugUtilsMessengerCreateInfoEXT& createInfo )
{
	createInfo = {};
	
	createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void	VulkanWindow::setupDebugMessenger ()
{
	if ( !enableValidationLayers ) 
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

	populateDebugMessengerCreateInfo ( createInfo );

	if ( createDebugUtilsMessengerEXT ( device.getInstance (), &createInfo, nullptr, &debugMessenger ) != VK_SUCCESS )
		fatal () << "VulkanWindow: failed to set up debug messenger!";
}

void	VulkanWindow::pickPhysicalDevice ()
{
	uint32_t deviceCount = 0;
	
	vkEnumeratePhysicalDevices ( device.getInstance (), &deviceCount, nullptr );

	if ( deviceCount == 0 )
		fatal () << "VulkanWindow: failed to find GPUs with Vulkan support!";

	std::vector<VkPhysicalDevice> devices ( deviceCount );

	vkEnumeratePhysicalDevices ( device.getInstance (), &deviceCount, devices.data () );

	for ( const auto& dev : devices )
		if ( isDeviceSuitable ( dev ) )
		{
			device.physicalDevice = dev;
			break;
		}

	if ( device.physicalDevice == VK_NULL_HANDLE )
		fatal () << "VulkanWindow: failed to find a suitable GPU!";
}

void	VulkanWindow::createLogicalDevice ()
{
	QueueFamilyIndices indices              = Device::findQueueFamilies ( device.getPhysicalDevice (), surface );
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	float queuePriority                     = 1.0f;
	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo createInfo           = {};

	queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
	queueCreateInfo.queueCount       = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;


	createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos       = &queueCreateInfo;
	createInfo.queueCreateInfoCount    = 1;
	createInfo.pEnabledFeatures        = &deviceFeatures;
	createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.enabledLayerCount       = 0;

	if ( enableValidationLayers )
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} 

	if ( vkCreateDevice ( device.getPhysicalDevice (), &createInfo, nullptr, &device.device ) != VK_SUCCESS )
		fatal () << "VulknaWindow: failed to create logical device!";

	device.graphicsFamilyIndex = indices.graphicsFamily;
	device.presentFamilyIndex  = indices.presentFamily;
	device.computeFamilyIndex  = indices.computeFamily;

	vkGetDeviceQueue ( device.getDevice (), indices.graphicsFamily, 0, &device.graphicsQueue );
	vkGetDeviceQueue ( device.getDevice (), indices.presentFamily,  0, &device.presentQueue  );
	vkGetDeviceQueue ( device.getDevice (), indices.computeFamily,  0, &device.computeQueue  );
}

void	VulkanWindow::createCommandPool ()
{
	QueueFamilyIndices		queueFamilyIndices = Device::findQueueFamilies ( device.getPhysicalDevice () );		// XXX: may be we should use graphicsQueue from  createLogicalDevice
	VkCommandPoolCreateInfo	poolInfo           = {};
		
	poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	if ( vkCreateCommandPool ( device.getDevice (), &poolInfo, nullptr, &device.commandPool ) != VK_SUCCESS )
		fatal () << "VulkanWindow: failed to create command pool!" << Log::endl;
}

void	VulkanWindow::createDepthTexture ()
{
	if ( hasDepth )
	{
		depthTexture.create ( device, getWidth (), getHeight (), 1, 1, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL, 
							  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		
		
		SingleTimeCommand	cmd ( device, device.getGraphicsQueue (), device.getCommandPool () );
			
		depthTexture.getImage ().transitionLayout ( cmd, depthTexture.getImage ().getFormat (), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ); 
	}
}

void	VulkanWindow::drawFrame ()
{
			// wait till we're safe to submit
	currentImage = swapChain.acquireNextImage ();
		
	if ( currentImage == UINT32_MAX )		// we need to recreate swap chain
	{
		recreateSwapChain ();
			
		return;
	}
				// submit command buffers
	submit ( currentImage );
		
				// actually present image
	swapChain.present ( currentImage, device.getPresentQueue () );
}

std::vector<const char*> VulkanWindow::getRequiredExtensions () const
{
	uint32_t      glfwExtensionCount = 0;
	const char ** glfwExtensions     = glfwGetRequiredInstanceExtensions ( &glfwExtensionCount );
		
	std::vector<const char*> extensions ( glfwExtensions, glfwExtensions + glfwExtensionCount );

	if ( enableValidationLayers )
		extensions.push_back ( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

	return extensions;
}

bool	VulkanWindow::checkValidationLayerSupport () const
{
	uint32_t layerCount;
		
	vkEnumerateInstanceLayerProperties ( &layerCount, nullptr );

	std::vector<VkLayerProperties> availableLayers ( layerCount );
		
	vkEnumerateInstanceLayerProperties ( &layerCount, availableLayers.data () );

	for ( const char * layerName : validationLayers )
	{
		bool layerFound = false;

		for ( const auto& layerProperties : availableLayers )
		{
			if ( strcmp ( layerName, layerProperties.layerName ) == 0 )
			{
				layerFound = true;
				break;
			}
		}

		if ( !layerFound )
			return false;
	}

	return true;
}

void	VulkanWindow :: updateFps ()
{
	for ( int i = 0; i < 5-1; i++ )
		frameTime [i] = frameTime [i+1];
		
	frameTime [5-1] = (float)getTime ();
	
	fps = 5 / (frameTime [5-1] - frameTime [0] + 0.00001f);		// add EPS to avoid zero division
	frame++;
	
	if ( showFps && (frame % 5 == 0) )			// update FPS every 5'th frame
	{
		char buf [64];
		
		sprintf ( buf, "%5.1f ", fps );
		
		setCaption ( std::string( buf ) + title );
	}
}

void	VulkanWindow::setFullscreen ( bool flag )
{
	if ( flag == fullScreen )
        return;

    if ( flag )
    {
			// backup window position and window size
        glfwGetWindowPos  ( window, &savePosX,  &savePosY   );
        glfwGetWindowSize ( window, &saveWidth, &saveHeight );

			// get resolution of monitor
		GLFWmonitor       * monitor = glfwGetPrimaryMonitor ();
        const GLFWvidmode * mode    = glfwGetVideoMode      ( monitor );
	
			// switch to full screen
        glfwSetWindowMonitor( window, monitor, 0, 0, mode->width, mode->height, 0 );
    }
    else
    {
			// restore last window size and position
        glfwSetWindowMonitor( window, nullptr,  savePosX, savePosY, saveWidth, saveHeight, 0 );
    }	
	
	fullScreen = flag;
}

bool	VulkanWindow::makeScreenshot ( const std::string& fileName ) const
{
	return false;
}

 VkResult VulkanWindow::createDebugUtilsMessengerEXT ( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger )
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if ( func != nullptr )
		return func ( instance, pCreateInfo, pAllocator, pDebugMessenger );

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void VulkanWindow::destroyDebugUtilsMessengerEXT ( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator )
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if ( func != nullptr )
		func ( instance, debugMessenger, pAllocator );
}

