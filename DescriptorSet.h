#pragma once

#include	<assert.h>
#include	"Buffer.h"
#include	"Texture.h"

class	DescriptorPool
{
	VkDescriptorPool 	descriptorPool = VK_NULL_HANDLE;
	Device			  * device         = nullptr;
	//VkDevice			device         = VK_NULL_HANDLE;
	
	std::vector<VkDescriptorPoolSize>	poolSizes = {};
	uint32_t							maxSets   = 0;
	
public:
	DescriptorPool () = default;
	~DescriptorPool ()
	{
		clean ();
	}
	
	VkDescriptorPool	getHandle () const
	{
		return descriptorPool;
	}
	
	void	clean ()
	{
		if ( descriptorPool != VK_NULL_HANDLE )
			vkDestroyDescriptorPool ( device->getDevice (), descriptorPool, nullptr );
		
		descriptorPool = VK_NULL_HANDLE;
	}

	DescriptorPool&	setMaxSets ( uint32_t count )
	{
		maxSets = count;
		
		return *this;
	}
	
	DescriptorPool&	setUniformBufferCount ( uint32_t count )
	{
		if ( count > 0 )
		{
			VkDescriptorPoolSize	size;
			
			size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			size.descriptorCount = count;
			
			poolSizes.push_back ( size );
		}
		
		return *this;
	}
	
	DescriptorPool&	setImageCount ( uint32_t count )
	{
		if ( count > 0 )
		{
			VkDescriptorPoolSize	size;
			
			size.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			size.descriptorCount = count;
			
			poolSizes.push_back ( size );
		}
		
		return *this;
	}
	
	DescriptorPool&	setStorageBufferCount ( uint32_t count )
	{
		if ( count > 0 )
		{
			VkDescriptorPoolSize	size;
			
			size.type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			size.descriptorCount = count;
			
			poolSizes.push_back ( size );
		}
		
		return *this;
	}
	
	void	create ( Device& dev )
	{	
		device = &dev;
		
		VkDescriptorPoolCreateInfo poolInfo = {};
		
		poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t) poolSizes.size ();
		poolInfo.pPoolSizes    = poolSizes.data();
		poolInfo.maxSets       = maxSets;

		if ( vkCreateDescriptorPool ( device->getDevice (), &poolInfo, nullptr, &descriptorPool ) != VK_SUCCESS )
			fatal () << "DescriptorPool: failed to create descriptor pool!";
	}
};

class	DescriptorSet
{
	Device							  * device              = nullptr;
	VkDescriptorSet						set                 = VK_NULL_HANDLE;
	VkDescriptorPool					descriptorPool      = VK_NULL_HANDLE;
	VkDescriptorSetLayout				descriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkWriteDescriptorSet>	writes;

public:
	DescriptorSet () = default;
	~DescriptorSet ()
	{
		clean ();
	}

	VkDescriptorSet	getHandle () const
	{
		return set;
	}

	void	clean ()
	{
		for ( auto& d : writes )
		{
			delete d.pBufferInfo;
			delete d.pImageInfo;
		}

		writes.clear ();
	}

	DescriptorSet&	setLayout (  Device& dev, VkDescriptorSetLayout descSetLayout, DescriptorPool& descPool )
	{
		device              = &dev;
		descriptorSetLayout = descSetLayout;
		descriptorPool      = descPool.getHandle ();

		return *this;
	}

	DescriptorSet&	addBuffer ( uint32_t binding, VkDescriptorType type, Buffer& buffer, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE )
	{
		if ( set == VK_NULL_HANDLE )
			alloc ();
		
		VkDescriptorBufferInfo * bufferInfo       = new VkDescriptorBufferInfo {};
		VkWriteDescriptorSet	 descriptorWrites = {};

		bufferInfo->buffer = buffer.getHandle ();
		bufferInfo->offset = offset;
		bufferInfo->range  = size;

		descriptorWrites.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites.dstSet          = set;
		descriptorWrites.dstBinding      = binding;
		descriptorWrites.dstArrayElement = 0;
		descriptorWrites.descriptorType  = type;	//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites.descriptorCount = 1;
		descriptorWrites.pBufferInfo     = bufferInfo;

		writes.push_back ( descriptorWrites );

		return *this;
	}

	DescriptorSet&	addImage ( uint32_t binding, VkDescriptorType type, Texture& texture, Sampler& sampler )
	{
		if ( set == VK_NULL_HANDLE )
			alloc ();
		
		assert ( texture.getImageView () != VK_NULL_HANDLE );
		assert ( sampler.getHandle    () != VK_NULL_HANDLE );

		VkDescriptorImageInfo    * imageInfo        = new VkDescriptorImageInfo {};
		VkWriteDescriptorSet	   descriptorWrites = {};

		imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo->imageView   = texture.getImageView ();
		imageInfo->sampler     = sampler.getHandle    ();

		descriptorWrites.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites.dstSet          = set;
		descriptorWrites.dstBinding      = binding;
		descriptorWrites.dstArrayElement = 0;
		descriptorWrites.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// type
		descriptorWrites.descriptorCount = 1;
		descriptorWrites.pImageInfo      = imageInfo;

		writes.push_back ( descriptorWrites );

		return *this;
	}

	void	alloc ()
	{
		assert ( device != nullptr && descriptorPool != VK_NULL_HANDLE && descriptorSetLayout != VK_NULL_HANDLE );
		
		VkDescriptorSetAllocateInfo allocInfo = {};

		allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool     = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts        = &descriptorSetLayout;

		if ( vkAllocateDescriptorSets ( device->getDevice (), &allocInfo, &set ) != VK_SUCCESS )
			fatal () << "DescriptorSet: failed to allocate descriptor sets!";
	}
	
	void	create ()
	{
		if ( set == VK_NULL_HANDLE )
			alloc ();
				
		vkUpdateDescriptorSets ( device->getDevice (), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr );
	}

};
