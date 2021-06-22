#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <vector>
#include <cstdint>
#include <array>

#include	"Log.h"
#include	"SwapChain.h"
#include	"Pipeline.h"
#include	"Texture.h"
#include	"Device.h"

class Buffer;
class Image;

class	VulkanWindow
{
protected:
	std::string			appName    = "Vulkan application";
	std::string			engineName = "No engine";
	GLFWwindow    	  * window     = nullptr;
	int					width, height;
	std::string			title;
	bool				hasDepth     = true;	// do we should attach depth buffer to FB
	uint32_t			currentImage = 0;
	bool				showFps      = false;
	bool				fullScreen   = false;
	int					frame        = 0;		// current frame nulber
	float				frameTime [5];			// time at last 5 frames for FPS calculations
	float				fps;
	int					savePosX, savePosY;		// save pos & size when going fullscreen
	int					saveWidth, saveHeight;	

	Device							device;
	VkDebugUtilsMessengerEXT		debugMessenger  = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT 		msgCallback     = VK_NULL_HANDLE;
	VkSurfaceKHR					surface         = VK_NULL_HANDLE;
	SwapChain						swapChain;
	Texture							depthTexture;

public:
	VulkanWindow ( int w, int h, const std::string& t, bool depth ) : hasDepth ( depth )
	{
		initWindow ( w, h, t );
		initVulkan ();	
	}

	~VulkanWindow ()
	{
		cleanup ();
	}

	void	setCaption ( const std::string& t )
	{
		title = t;
		glfwSetWindowTitle ( window, title.c_str () );
	}
	
	std::string	getCaption () const
	{
		return title;
	}
	
	double	getTime () const	// return time in seconds 
	{
		return glfwGetTime ();
	}

	GLFWwindow * getWindow () const
	{
		return window;
	}

	uint32_t	getWidth () const
	{
		return swapChain.getExtent ().width;
	}
	
	uint32_t	getHeight () const
	{
		return swapChain.getExtent ().height;
	}
	
	void	setSize ( uint32_t w, uint32_t h )
	{
		glfwSetWindowSize ( window, width = w, height = h );
	}
	
	float	getAspect () const
	{
		return static_cast<float> ( getWidth () ) / static_cast<float> ( getHeight () );
	}

	void	setShowFps ( bool flag )
	{
		showFps = flag;
	}
	
	void	setFullscreen ( bool flag );
	
	bool	makeScreenshot ( const std::string& fileName ) const;

	virtual	int	run ()
	{
		mainLoop ();

		return EXIT_SUCCESS;
	}

	virtual void initWindow ( int w, int h, const std::string& t );
	virtual void initVulkan ();
	virtual	void cleanup    ();		// clean all Vulkan objects
	
	virtual void mainLoop   ();

			// cleanup swap chain objects due to window resize
	void cleanupSwapChain ()
	{
		depthTexture.clean ();
		
				// clean up objects in upper classes
		freePipelines ();

				// swapChain.cleanup without destroying sync objects
		swapChain.cleanup ( false );
	}

			// recreate swap chain objects due to window resize
	virtual void recreateSwapChain  ();

	bool	isDeviceSuitable ( VkPhysicalDevice device )
	{
		QueueFamilyIndices indices = Device::findQueueFamilies ( device, surface );

		return indices.isComplete ();
	}

	virtual	void createInstance     ();
	virtual	void pickPhysicalDevice ();

	void	createLogicalDevice ();
	void	createCommandPool   ();
	void	createDepthTexture  ();
	
	void	createSurface ()
	{
		if ( glfwCreateWindowSurface ( device.getInstance (), window, nullptr, &surface ) != VK_SUCCESS )
			fatal () << "VulkanWindow: failed to create window surface!";
	}

	void	destroyCommandPool ()
	{
		vkDestroyCommandPool ( device.getDevice (), device.getCommandPool (), nullptr );
		
		device.commandPool = VK_NULL_HANDLE;
	}

				// create pipelines, renderpasses, command buffers and descriptor sets
				// May be called several times due to window size changes
	virtual	void	createPipelines () {}
	
				// free them when close or change window size
	virtual	void	freePipelines   () {}
	
	virtual	void	drawFrame ();
	virtual	void	submit    ( uint32_t imageIndex ) {}			// perform actual sumitting of rendering 
	
				// window events
	virtual	void	reshape     ( int w, int h ) {}
	virtual	void	keyTyped    ( int key, int scancode, int action, int mods ) {}
	virtual	void	mouseMotion ( double x, double y ) {}
	virtual	void	mouseClick  ( int button, int action, int mods ) {}
	virtual	void	mouseWheel  ( double xOffset, double yOffset ) {}
	virtual	void	idle        () {}
	
private:
	void	updateFps ();
	
	std::vector<const char*> getRequiredExtensions () const;
	bool checkValidationLayerSupport () const;

			// Vulkan callbacks
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback ( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData )
	{
		log () << "validation layer: " << pCallbackData->pMessage << Log::endl;

		return VK_FALSE;
	}
	
	virtual	void setupDebugMessenger ();
	virtual	void populateDebugMessengerCreateInfo ( VkDebugUtilsMessengerCreateInfoEXT& createInfo );
	
	static VkResult createDebugUtilsMessengerEXT ( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger );
	static void destroyDebugUtilsMessengerEXT    ( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator );
};

		// return normal matrix for a given model-view
inline glm::mat3 normalMatrix ( const glm::mat4& modelView ) 
{
	return glm::inverseTranspose ( glm::mat3 ( modelView ) );
}

inline glm::mat4 projectionMatrix ( float fov, float aspect, float zMin, float zMax )
{
	glm::mat4	proj = glm::perspective ( fov, aspect, zMin, zMax );
	
	proj [1][1] *= -1;
	
	return proj;
}
