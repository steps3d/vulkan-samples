#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include	"Log.h"
#include	"SingleTimeCommand.h"
#include	"Device.h"

class GpuMemory
{
	Device		  * device = nullptr;
	VkDeviceMemory	memory = VK_NULL_HANDLE;
	VkDeviceSize	size   = 0;

public:
	GpuMemory () {}
	GpuMemory ( Device& dev, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties )
	{
		alloc ( dev, memRequirements, properties );
	}

	~GpuMemory () 
	{
		clean ();
	}

	VkDevice	getDevice () const
	{
		return device->getDevice ();
	}

	VkPhysicalDevice	getPhysicalDevice () const
	{
		return device->getPhysicalDevice ();
	}

	VkDeviceMemory	getMemory () const
	{
		return memory;
	}

	VkDeviceSize	getSize () const
	{
		return size;
	}

	void	clean ()
	{
		if ( memory != VK_NULL_HANDLE )
			vkFreeMemory ( device->getDevice (), memory, nullptr );

		memory = VK_NULL_HANDLE;
	}

	bool	alloc ( Device& dev, VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties )
	{
		VkMemoryAllocateInfo allocInfo = {};
		
		device                    = &dev;
		size                      = memRequirements.size;
		allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize  = size;
		allocInfo.memoryTypeIndex = findMemoryType ( memRequirements.memoryTypeBits, properties );

		return vkAllocateMemory ( device->getDevice (), &allocInfo, nullptr, &memory ) == VK_SUCCESS;
	}

	uint32_t findMemoryType ( uint32_t typeFilter, VkMemoryPropertyFlags properties ) const
	{
		VkPhysicalDeviceMemoryProperties 	memProperties;
		
		vkGetPhysicalDeviceMemoryProperties ( device->getPhysicalDevice (), &memProperties );

		for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
			if ( (typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties ) 
				return i;

		fatal () << "GpuMemory: failed to find suitable memory type! " << properties;

		return 0;		// we won't get here, but compiler complains about returning no value
	}

	bool	copy ( const void * ptr, VkDeviceSize size, size_t offs = 0 )
	{
		assert ( offs + size <= this->size );

		void * data;

		if ( !memory )
			return false;

		vkMapMemory   ( device->getDevice (), memory, 0, size, 0, &data );
		memcpy        ( offs + (char *) data,  ptr, size );
		vkUnmapMemory ( device->getDevice (), memory );

		// vkInvalidateMappedMemoryRanges if not host coherent
		return true;
	}
	
	void * map ( VkDeviceSize size, size_t offs = 0 )
	{
		void * data;
		
		vkMapMemory ( device->getDevice (), memory, 0, size, 0, &data );
		
		return data;
	}
	
	void	unmap () 
	{
		vkUnmapMemory ( device->getDevice (), memory );
	}
};

class Buffer
{
	VkBuffer	buffer = VK_NULL_HANDLE;
	GpuMemory	memory;

public:
	Buffer () {}
	Buffer ( Device& dev, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties )
	{
		create ( dev, size, usage, properties );
	}

	~Buffer () 
	{
		clean ();
	}

	VkDevice	getDevice () const
	{
		return memory.getDevice ();
	}

	GpuMemory&	getMemory ()
	{
		return memory;
	}

	VkBuffer	getHandle () const
	{
		return buffer;
	}
	
	void	clean ()
	{
		if ( buffer != VK_NULL_HANDLE )
		        vkDestroyBuffer ( memory.getDevice (), buffer, nullptr );

		buffer = VK_NULL_HANDLE;

		memory.clean ();
	}

	bool	create ( Device& dev, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties )
	{
		VkBufferCreateInfo		bufferInfo = {};
		VkMemoryRequirements	memRequirements;

		bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size        = size;
		bufferInfo.usage       = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if ( vkCreateBuffer ( dev.getDevice (), &bufferInfo, nullptr, &buffer ) != VK_SUCCESS )
			fatal () << "Buffer: failed to create buffer!";

		vkGetBufferMemoryRequirements ( dev.getDevice (), buffer, &memRequirements );

		if ( !memory.alloc ( dev, memRequirements, properties ) )
			fatal () << "Buffer: cannot allocate memorry";

		vkBindBufferMemory ( dev.getDevice (), buffer, memory.getMemory (), 0 );

		return true;
	}

	bool	copy ( const void * ptr, VkDeviceSize size, size_t offs = 0 )
	{
		return memory.copy ( ptr, size, offs );
	}

	void	copyBuffer ( SingleTimeCommand& cmd, Buffer& fromBuffer, VkDeviceSize size )
	{
		VkBufferCopy	copyRegion    = {};

		copyRegion.size = size;

		vkCmdCopyBuffer ( cmd.getHandle (), fromBuffer.getHandle (), getHandle (), 1, &copyRegion );
	}
};
