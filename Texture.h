
#pragma once

#include	"Buffer.h"
#include	"SingleTimeCommand.h"
#include	"Device.h"
#include	"stb_image_aug.h"

#include	<algorithm>			// for std::max
#include	<cmath>				// for log2
#include	<vector>

#undef	min
#undef	max

class	ImageCreateInfo
{
	VkImageCreateInfo imageInfo = {};
	
public:
	ImageCreateInfo ( uint32_t width, uint32_t height, uint32_t depth = 1 )
	{
		imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.extent.width  = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth  = depth;
		imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.imageType     = VK_IMAGE_TYPE_3D;
		imageInfo.arrayLayers   = 1;
		imageInfo.mipLevels     = 1;
		
		if ( height == 1 && depth == 1 )
			imageInfo.imageType = VK_IMAGE_TYPE_1D;
		else
		if ( depth == 1 )
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
	}
	
	ImageCreateInfo&	setFormat ( VkFormat format )
	{
		imageInfo.format = format;
		return *this;
	}

	ImageCreateInfo&	setTiling ( VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL )
	{
		imageInfo.tiling = tiling;
		return *this;
	}
	
	ImageCreateInfo&	setMipLevels ( uint32_t mipLevels )
	{
		imageInfo.mipLevels = mipLevels;
		return *this;
	}
		
	ImageCreateInfo&	setLayers ( uint32_t layers )
	{
		imageInfo.arrayLayers = layers;
		return *this;
	}
	
	ImageCreateInfo&	setInitialLayer ( VkImageLayout layout )
	{
		imageInfo.initialLayout = layout;
		return *this;
	}

	ImageCreateInfo&	setUsage ( VkImageUsageFlags flags )
	{
		imageInfo.usage = flags;
		return *this;
	}
	
	ImageCreateInfo&	setFlags ( VkImageCreateFlags flags )
	{
		imageInfo.flags = flags;
		return *this;
	}

	const VkImageCreateInfo * data () const
	{
		return &imageInfo;
	}
};

class Image 
{
	int					width, height, depth;
	GpuMemory			memory;
	VkImage				image       = VK_NULL_HANDLE;
	VkImageType			type        = VK_IMAGE_TYPE_2D;
	uint32_t			mipLevels   = 1;
	uint32_t			arrayLayers = 1;
	VkImageTiling		tiling      = VK_IMAGE_TILING_OPTIMAL;
	VkFormat			format      = VK_FORMAT_R8G8B8A8_UNORM;
	
public:
	Image  () = default;
	~Image ()
	{
		clean ();
	}

	void	clean ()
	{
		if ( image != VK_NULL_HANDLE )
	        	vkDestroyImage ( memory.getDevice (), image, nullptr );

		image = VK_NULL_HANDLE;

		memory.clean ();
	}

	uint32_t	getWidth () const
	{
		return width;
	}
	
	uint32_t	getHeight () const
	{
		return height;
	}
	
	uint32_t	getDepth () const
	{
		return depth;
	}

	uint32_t	getMipLevels () const
	{
		return mipLevels;
	}
	
	VkImage	getHandle () const
	{
		return image;
	}
	
	VkDevice	getDevice () const
	{
		return memory.getDevice ();
	}

	VkPhysicalDevice	getPhysicalDevice () const
	{
		return memory.getPhysicalDevice ();
	}

	VkFormat	getFormat () const
	{
		return format;
	}
	
	GpuMemory&	getMemory ()
	{
		return memory;
	}

	bool hasDepth () const
	{
		std::vector<VkFormat> formats = 
		{
			VK_FORMAT_D16_UNORM,
			VK_FORMAT_X8_D24_UNORM_PACK32,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
		};
		
		return std::find(formats.begin(), formats.end(), format ) != std::end(formats);
	}

	bool hasStencil () const
	{
		std::vector<VkFormat> formats = 
		{
			VK_FORMAT_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
		};
		
		return std::find ( formats.begin(), formats.end(), format ) != std::end(formats);
	}

	bool isDepthStencil ()
	{
		return hasDepth () || hasStencil ();
	}


	bool	create ( Device& dev, uint32_t w, uint32_t h, uint32_t d, uint32_t numMipLevels, VkFormat fmt, VkImageTiling tl, 
					 VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED )
	{
		VkImageCreateInfo imageInfo = {};

		width                   = w;
		height                  = h;
		depth                   = d;
		format                  = fmt;
		tiling                  = tl;
		mipLevels               = numMipLevels;
		imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType     = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width  = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth  = 1;
		imageInfo.mipLevels     = mipLevels;
		imageInfo.arrayLayers   = 1;
		imageInfo.format        = format;
		imageInfo.tiling        = tiling;
		imageInfo.initialLayout = initialLayout;
		imageInfo.usage         = usage;
		imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

		if ( vkCreateImage ( dev.getDevice (), &imageInfo, nullptr, &image ) != VK_SUCCESS ) 
			fatal () << "Image: Cannot create image";

		VkMemoryRequirements memRequirements;

		vkGetImageMemoryRequirements ( dev.getDevice (), image, &memRequirements );

		if ( !memory.alloc ( dev, memRequirements, properties ) )
			fatal () << "Image: Cannot alloc memory for image";

		vkBindImageMemory ( dev.getDevice (), image, memory.getMemory (), 0 );

		return true;
	}

	bool	create ( Device& dev, ImageCreateInfo& info, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT )
	{
		width                   = info.data ()->extent.width;
		height                  = info.data ()->extent.height;
		depth                   = info.data ()->extent.depth;
		format                  = info.data ()->format;
		tiling                  = info.data ()->tiling;
		mipLevels               = info.data ()->mipLevels;
		arrayLayers             = info.data ()->arrayLayers;

		if ( vkCreateImage ( dev.getDevice (), info.data (), nullptr, &image ) != VK_SUCCESS ) 
			fatal () << "Image: Cannot create image";

		VkMemoryRequirements memRequirements;

		vkGetImageMemoryRequirements ( dev.getDevice (), image, &memRequirements );

		if ( !memory.alloc ( dev, memRequirements, properties ) )
			fatal () << "Image: Cannot alloc memory for image";

		vkBindImageMemory ( dev.getDevice (), image, memory.getMemory (), 0 );

		return true;
	}
	
	void	transitionLayout ( SingleTimeCommand& cmd, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout )
	{
		VkImageMemoryBarrier	barrier       = {};
		VkPipelineStageFlags 	sourceStage;
		VkPipelineStageFlags 	destinationStage;

		barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout                       = oldLayout;
		barrier.newLayout                       = newLayout;
		barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.image                           = image;
		barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel   = 0;
		barrier.subresourceRange.levelCount     = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount     = arrayLayers;

		if ( newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if ( format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT )
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		} 
		else
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		
		if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL )
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			sourceStage           = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else
		if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage           = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} 
		else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage           = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage      = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} 
		else if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			sourceStage           = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage      = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
			fatal () << "Texture: Unsupported layout transition!";

		vkCmdPipelineBarrier (
			cmd.getHandle (),
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier );
	}

	void	copyFromBuffer ( SingleTimeCommand& cmd, Buffer& buffer, uint32_t width, uint32_t height, uint32_t depth = 1, uint32_t layers = 1, uint32_t mipLevel = 0 )
	{
		VkBufferImageCopy	region        = {};
		
		region.bufferOffset                    = 0;
		region.bufferRowLength                 = 0;
		region.bufferImageHeight               = 0;
		region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel       = mipLevel;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount     = layers;
		region.imageOffset                     = {0, 0, 0};
		region.imageExtent                     = { width, height, depth };

		vkCmdCopyBufferToImage ( cmd.getHandle (), buffer.getHandle (), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
	}


	static uint32_t	calcNumMipLevels ( uint32_t w, uint32_t h, uint32_t d = 1 )
	{
		auto	size = std::max ( std::max ( w, h ), d );
		
		return static_cast<uint32_t>( std::floor ( std::log2 ( size ) ) ) + 1;
	}
	
	static void	transitionLayout ( SingleTimeCommand& cmd, VkImage image, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, 
								   VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout )
	{
		VkImageMemoryBarrier	barrier       = {};

		barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout                       = oldLayout;
		barrier.newLayout                       = newLayout;
		barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.image                           = image;
		barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel   = 0;
		barrier.subresourceRange.levelCount     = 1;	// XXX
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount     = 1;	// XXX
		barrier.srcAccessMask                   = srcAccessMask;
		barrier.dstAccessMask                   = dstAccessMask;

		vkCmdPipelineBarrier (
			cmd.getHandle (),
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier );
	}
};

class	Sampler
{
	VkDevice				device        = VK_NULL_HANDLE;
	VkSampler				sampler       = VK_NULL_HANDLE;
	VkFilter				minFilter     = VK_FILTER_NEAREST;	//LINEAR;
	VkFilter				magFilter     = VK_FILTER_NEAREST;	//LINEAR;
	VkSamplerAddressMode	addressModeU  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeV  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode	addressModeW  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkBool32				anisotropy    = VK_FALSE;
	float					maxAnisotropy = 1.0f;		// 16
	VkBool32				unnormalized  = VK_FALSE;
	VkSamplerMipmapMode		mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	float					lodBias       = 0;
	float					minLod        = 0;
	float					maxLod        = 0;
	
public:
	Sampler () {}
	~Sampler ()
	{
		clean ();
	}
	
	VkSampler	getHandle () const
	{
		return sampler;
	}
	
	void	clean ()
	{
		if ( sampler != VK_NULL_HANDLE )
	        vkDestroySampler ( device, sampler, nullptr );
		
		sampler = VK_NULL_HANDLE;
	}

	Sampler&	setMinFilter ( VkFilter filter )
	{
		minFilter = filter;
		return *this;
	}
	
	Sampler&	setMagFilter ( VkFilter filter )
	{
		magFilter = filter;
		return *this;
	}
	
	Sampler&	setAddressMode ( VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w )
	{
		addressModeU = u;
		addressModeV = v;
		addressModeW = w;
		return *this;
	}
	
	Sampler&	setAnisotropy ( bool enable, float maxAniso = 0 )
	{
		anisotropy    = enable ? VK_TRUE : VK_FALSE;
		maxAnisotropy = maxAniso;
		return *this;
	}
	
	Sampler&	setNormalized ( bool flag )
	{
		unnormalized = flag ? VK_FALSE : VK_TRUE;
		return *this;
	}
	
	Sampler&	setMipmapMode ( VkSamplerMipmapMode mode )
	{
		mipmapMode = mode;
		return *this;
	}
	
	Sampler&	setMipmapBias ( float bias )
	{
		lodBias = bias;
		return *this;
	}
	
	Sampler&	setMinLod ( float v )
	{
		minLod = v;
		return *this;
	}
	
	Sampler&	setMaxLod ( float v )
	{
		maxLod = v;
		return *this;
	}
	
	void create ( Device& dev ) 
	{
		device = dev.getDevice ();
		
		VkSamplerCreateInfo samplerInfo = {};
	
		samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter               = magFilter;
		samplerInfo.minFilter               = minFilter;
		samplerInfo.addressModeU            = addressModeU;
		samplerInfo.addressModeV            = addressModeV;
		samplerInfo.addressModeW            = addressModeW;
		samplerInfo.anisotropyEnable        = anisotropy;
		samplerInfo.maxAnisotropy           = maxAnisotropy;
		samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = unnormalized;
		samplerInfo.compareEnable           = VK_FALSE;
		samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode              = mipmapMode;
		samplerInfo.mipLodBias              = lodBias;
		samplerInfo.minLod                  = minLod;
		samplerInfo.maxLod                  = maxLod;

        if ( vkCreateSampler ( device, &samplerInfo, nullptr, &sampler ) != VK_SUCCESS )
			fatal () << "Sampler: failed to create texture sampler!";
	}
};

class Texture
{
	Image			image;
	VkImageView 	imageView = VK_NULL_HANDLE;

public:
	Texture () = default;
	~Texture () 
	{
		clean ();
	}
	
	void	clean ()
	{
		image.clean ();
		
		if ( imageView != VK_NULL_HANDLE )
			vkDestroyImageView ( image.getDevice (), imageView, nullptr );
		
		imageView = VK_NULL_HANDLE;
	}

	Image&	getImage ()
	{
		return image;
	}

	VkImageView	getImageView () const
	{
		return imageView;
	}

	uint32_t	getWidth () const
	{
		return image.getWidth ();
	}

	uint32_t	getHeight () const
	{
		return image.getHeight ();
	}

	uint32_t	getDepth () const
	{
		return image.getDepth ();
	}

	VkFormat	getFormat () const
	{
		return image.getFormat ();
	}

	void	createImageView ( VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D )
	{
		assert ( image.getHandle () != VK_NULL_HANDLE );
		
		VkImageViewCreateInfo viewInfo = {};
		
		viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image                           = image.getHandle ();
		viewInfo.viewType                        = viewType;
		viewInfo.format                          = image.getFormat ();
		viewInfo.subresourceRange.aspectMask     = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel   = 0;
		viewInfo.subresourceRange.levelCount     = image.getMipLevels ();
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount     = 1;

		if ( vkCreateImageView ( image.getDevice (), &viewInfo, nullptr, &imageView ) != VK_SUCCESS )
			fatal () << "Texture: failed to create texture image view!";
	}

	Texture&	create ( Device& dev, uint32_t w, uint32_t h, uint32_t d, uint32_t mipLevels, VkFormat fmt, VkImageTiling tl, 
						 VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED )
	{
		image.create    ( dev, w, h, d, mipLevels, fmt, tl, usage, properties );
		
				// check for depth image
		VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		
		if ( fmt == VK_FORMAT_D32_SFLOAT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT || fmt == VK_FORMAT_D24_UNORM_S8_UINT )
			aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		
		createImageView ( aspectFlags );
		
		return *this;
	}
	
	
	void	generateMipmaps ( SingleTimeCommand& cmd, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels ) 
	{
			// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		
		vkGetPhysicalDeviceFormatProperties ( image.getPhysicalDevice (), imageFormat, &formatProperties );

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT ) )
			fatal () << "Texture: texture image format does not support linear blitting!";

		VkImageMemoryBarrier	barrier   = {};
		int32_t					mipWidth  = image.getWidth  ();
		int32_t					mipHeight = image.getHeight ();
		
		barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image                           = image.getHandle ();
		barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount     = 1;
		barrier.subresourceRange.levelCount     = 1;


		for ( uint32_t i = 1; i < mipLevels; i++ ) 
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier ( cmd.getHandle (),
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr, 0, nullptr, 1, &barrier );

			VkImageBlit blit = {};
			
			blit.srcOffsets[0]                 = {0, 0, 0};
			blit.srcOffsets[1]                 = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel       = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount     = 1;
			blit.dstOffsets[0]                 = {0, 0, 0};
			blit.dstOffsets[1]                 = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel       = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount     = 1;

			vkCmdBlitImage ( cmd.getHandle (),
				image.getHandle (), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image.getHandle (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit, VK_FILTER_LINEAR );

			barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier ( cmd.getHandle (),
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr, 0, nullptr, 1, &barrier );

			if ( mipWidth > 1 )
				mipWidth /= 2;
				
			if ( mipHeight > 1 )
				mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier ( cmd.getHandle (),
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr, 0, nullptr, 1, &barrier );
	}
	
	void load2D ( Device& dev, const std::string& fileName, bool mipmaps = true )
	{
		int				texWidth, texHeight, texChannels;
		stbi_uc       * pixels    = stbi_load ( fileName.c_str (), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
		VkDeviceSize	imageSize = texWidth * texHeight * 4;
		uint32_t		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		if ( !pixels )
			fatal () << "Texture: failed to load texture image! " << fileName << Log::endl;

		Buffer	stagingBuffer;
		
		stagingBuffer.create ( dev, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		stagingBuffer.copy   ( pixels, imageSize );

		stbi_image_free ( pixels );

			// TRANSFER_SRC for mipmap calculations via vkCmdBlitImage
		create ( dev, texWidth, texHeight, 1, mipLevels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		{
			SingleTimeCommand	cmd ( dev );
			
			image.transitionLayout  ( cmd, image.getFormat (), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
			image.copyFromBuffer    ( cmd, stagingBuffer, texWidth, texHeight, 1 );
			
			if ( mipmaps )
				generateMipmaps     ( cmd, image.getFormat (), texWidth, texHeight, mipLevels );
			else
				image.transitionLayout ( cmd, image.getFormat (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
		}
	}

	void	loadCubemap ( Device& dev, const std::vector<const char *>& files, bool mipmaps = true )
	{
		assert ( files.size () == 6 );

		struct
		{
			int			width, height, numChannels;
			stbi_uc   * pixels;
		} faces [6];
		
		for ( int i = 0; i < 6; i++ )
		{
			faces [i].pixels = stbi_load ( files [i], &faces [i].width, &faces [i].height, &faces [i].numChannels, STBI_rgb_alpha );
			
			assert ( faces [i].width == faces [i].height );
			assert ( faces [i].pixels != nullptr );
		}
		
		assert ( faces [0].width == faces [1].width && faces [2].width == faces [3].width && faces [4].width == faces [5].width && faces [1].width == faces [2].width && faces [3].width == faces [4].width );
		
		auto			width     = faces [0].width;
		VkDeviceSize	faceSize  = width * width * 4;
		VkDeviceSize	imageSize = faceSize * 6;
		uint32_t		mipLevels = static_cast<uint32_t>(std::floor(std::log2(width))) + 1;
		Buffer			stagingBuffer;
		
		if ( !mipmaps )
			mipLevels = 1;
		
				// copy all data to staging buffer
		stagingBuffer.create ( dev, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		
		for ( int i = 0; i < 6; i++ )
		{
			stagingBuffer.copy   ( faces [i].pixels, faceSize, i*faceSize );

			stbi_image_free ( faces [i].pixels );
		}
		
			// create layered 2D image with 6 layers
		image.create ( dev, ImageCreateInfo ( width, width ).setFlags ( VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT ).setFormat ( VK_FORMAT_R8G8B8A8_UNORM ).setMipLevels ( mipLevels ).setLayers ( 6 ).setUsage ( VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ) );
		
				// check for depth image
		VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		
		if ( image.getFormat () == VK_FORMAT_D32_SFLOAT || image.getFormat () == VK_FORMAT_D32_SFLOAT_S8_UINT || image.getFormat () == VK_FORMAT_D24_UNORM_S8_UINT )
			aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		
		{
			SingleTimeCommand	cmd ( dev );
			
			image.transitionLayout  ( cmd, image.getFormat (), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
			image.copyFromBuffer    ( cmd, stagingBuffer, width, width, 1, 6 );

			if ( mipmaps )
				generateMipmaps     ( cmd, image.getFormat (), width, width, mipLevels );
			else
				image.transitionLayout ( cmd, image.getFormat (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
		}
		
		createImageView ( aspectFlags, VK_IMAGE_VIEW_TYPE_CUBE );
	}
};

inline bool	isDepthFormat ( VkFormat format )
{
	const std::vector<VkFormat> formats = 
	{
		VK_FORMAT_D16_UNORM,
		VK_FORMAT_X8_D24_UNORM_PACK32,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
	};
		
	return std::find ( formats.begin(), formats.end(), format ) != std::end(formats);
}
