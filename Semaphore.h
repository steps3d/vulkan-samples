
#pragma once

class	Semaphore 
{
	VkSemaphore handle = VK_NULL_HANDLE;
	Device   * device  = nullptr;

public:
	Semaphore  () = default;
	~Semaphore ()
	{
		clean ();
	}

	VkSemaphore	getHandle () const
	{
		return handle;
	}

	void	clean ()
	{
		vkDestroySemaphore ( device->getDevice (), handle, nullptr );

		handle = VK_NULL_HANDLE;
	}

	void	create ( Device& dev )
	{
		device = &dev;

		VkSemaphoreCreateInfo semaphoreCreateInfo = {};

		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if ( vkCreateSemaphore ( device->getDevice (), &semaphoreCreateInfo, nullptr, &handle ) != VK_SUCCESS )
			fatal () << "Semaphore: error creating" << Log::endl;
	}


	void	signal ( VkQueue queue )
	{
		VkSubmitInfo	submitInfo = {};
		
		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = &handle;
		
		vkQueueSubmit   ( queue, 1, &submitInfo, VK_NULL_HANDLE );
		vkQueueWaitIdle ( queue );
	}
};

class	Fence
{
	VkFence		fence               = VK_NULL_HANDLE;
	Device    * device  = nullptr;
	
public:
	Fence () = default;
	~Fence ()
	{
		clean ();
	}
	
	VkFence	getHandle () const
	{
		return fence;
	}
	
	void	clean ()
	{
		if ( fence != VK_NULL_HANDLE )
			vkDestroyFence  ( device->getDevice (), fence, nullptr );

		fence = VK_NULL_HANDLE;
	}
	
	void	create ( Device& dev )
	{
		VkFenceCreateInfo fenceInfo = {};

		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		device          = &dev;
		
		vkCreateFence ( device->getDevice (), &fenceInfo, nullptr, &fence );
	}

		// set state to unsignalled from host
	void	reset ()
	{
		vkResetFences ( device->getDevice (), 1, &fence );
	}

		// VK_SUCCESS - signaled
		// VK_NOT_READY unsignaled
	VkResult	status () const
	{
		return vkGetFenceStatus ( device->getDevice (), fence );
	}

		// wait for fence, timeout in nanoseconds
	bool	wait ( uint64_t timeout )
	{
		return vkWaitForFences ( device->getDevice (), 1, &fence, VK_TRUE, timeout ) == VK_SUCCESS;
	}
};
