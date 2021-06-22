#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include	"Log.h"
#include	"Data.h"
#include	"Device.h"
#include	"Texture.h"

#pragma once

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0,ch1,ch2,ch3)				\
  ((uint32_t)((uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | 		\
   ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24)))
#endif /* MAKEFOURCC */

#pragma pack (push, 1)

struct DdsHeader
{
	uint32_t	dwMagic;
	uint32_t	dwSize;
	uint32_t	dwFlags;
	uint32_t	dwHeight;
	uint32_t	dwWidth;
	uint32_t	dwPitchOrLinearSize;
	uint32_t	dwDepth;
	uint32_t	dwMipMapCount;
	uint32_t	dwReserved1 [11];

		//  DDPIXELFORMAT
	struct 
	{
		uint32_t	dwSize;
		uint32_t	dwFlags;
		uint32_t	dwFourCC;
		uint32_t	dwRGBBitCount;
		uint32_t	dwRBitMask;
		uint32_t	dwGBitMask;
		uint32_t	dwBBitMask;
		uint32_t	dwAlphaBitMask;
    } sPixelFormat;

		//  DDCAPS2
	struct 
	{
		uint32_t	dwCaps1;
		uint32_t	dwCaps2;
		uint32_t	dwDDSX;
		uint32_t	dwReserved;
	} sCaps;
	
	uint32_t	dwReserved2;
};

struct Dx10Header
{
	//DXGI_FORMAT              dxgiFormat;
	uint32_t				 dxgiFormat;
	//D3D10_RESOURCE_DIMENSION resourceDimension;
	uint32_t				resourceDimension;
	uint32_t                 miscFlag;
	uint32_t                 arraySize;
	uint32_t                 miscFlags2;
} DDS_HEADER_DXT10;

enum D3D10_RESOURCE_DIMENSION 
{
	D3D10_RESOURCE_DIMENSION_UNKNOWN,		// 0 
	D3D10_RESOURCE_DIMENSION_BUFFER,		// 1
	D3D10_RESOURCE_DIMENSION_TEXTURE1D,		// 2
	D3D10_RESOURCE_DIMENSION_TEXTURE2D,		// 3
	D3D10_RESOURCE_DIMENSION_TEXTURE3D		// 4
};

enum DXGI_FORMAT 
{
	DXGI_FORMAT_UNKNOWN,					// 0
	DXGI_FORMAT_R32G32B32A32_TYPELESS,		// 1
	DXGI_FORMAT_R32G32B32A32_FLOAT,			// 2
	DXGI_FORMAT_R32G32B32A32_UINT,			// 3
	DXGI_FORMAT_R32G32B32A32_SINT,			// 4
	DXGI_FORMAT_R32G32B32_TYPELESS,			// 5
	DXGI_FORMAT_R32G32B32_FLOAT,			// 6
	DXGI_FORMAT_R32G32B32_UINT,				// 7
	DXGI_FORMAT_R32G32B32_SINT,				// 8
	DXGI_FORMAT_R16G16B16A16_TYPELESS,		// 9
	DXGI_FORMAT_R16G16B16A16_FLOAT,			// 10
	DXGI_FORMAT_R16G16B16A16_UNORM,			// 11
	DXGI_FORMAT_R16G16B16A16_UINT,			// 12
	DXGI_FORMAT_R16G16B16A16_SNORM,			// 13
	DXGI_FORMAT_R16G16B16A16_SINT,			// 14
	DXGI_FORMAT_R32G32_TYPELESS,			// 15
	DXGI_FORMAT_R32G32_FLOAT,				// 16
	DXGI_FORMAT_R32G32_UINT,				// 17
	DXGI_FORMAT_R32G32_SINT,				// 18
	DXGI_FORMAT_R32G8X24_TYPELESS,			// 19
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT,		// 20
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,	// 21
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,	// 22
	DXGI_FORMAT_R10G10B10A2_TYPELESS,		// 23
	DXGI_FORMAT_R10G10B10A2_UNORM,			// 24
	DXGI_FORMAT_R10G10B10A2_UINT,			// 25
	DXGI_FORMAT_R11G11B10_FLOAT,			// 26
	DXGI_FORMAT_R8G8B8A8_TYPELESS,			// 27
	DXGI_FORMAT_R8G8B8A8_UNORM,				// 28
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,		// 29
	DXGI_FORMAT_R8G8B8A8_UINT,				// 30
	DXGI_FORMAT_R8G8B8A8_SNORM,				// 31
	DXGI_FORMAT_R8G8B8A8_SINT,				// 32
	DXGI_FORMAT_R16G16_TYPELESS,			// 33
	DXGI_FORMAT_R16G16_FLOAT,				// 34
	DXGI_FORMAT_R16G16_UNORM,				// 35
	DXGI_FORMAT_R16G16_UINT,				// 36
	DXGI_FORMAT_R16G16_SNORM,				// 37
	DXGI_FORMAT_R16G16_SINT,				// 38
	DXGI_FORMAT_R32_TYPELESS,				// 39
	DXGI_FORMAT_D32_FLOAT,					// 40
	DXGI_FORMAT_R32_FLOAT,					// 41
	DXGI_FORMAT_R32_UINT,					// 42
	DXGI_FORMAT_R32_SINT,					// 43
	DXGI_FORMAT_R24G8_TYPELESS,				// 44
	DXGI_FORMAT_D24_UNORM_S8_UINT,			// 45
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,		// 46
	DXGI_FORMAT_X24_TYPELESS_G8_UINT,		// 47
	DXGI_FORMAT_R8G8_TYPELESS,				// 48
	DXGI_FORMAT_R8G8_UNORM,					// 49
	DXGI_FORMAT_R8G8_UINT,					// 50
	DXGI_FORMAT_R8G8_SNORM,					// 51
	DXGI_FORMAT_R8G8_SINT,					// 52
	DXGI_FORMAT_R16_TYPELESS,				// 53
	DXGI_FORMAT_R16_FLOAT,					// 54
	DXGI_FORMAT_D16_UNORM,					// 55
	DXGI_FORMAT_R16_UNORM,					// 56
	DXGI_FORMAT_R16_UINT,					// 57
	DXGI_FORMAT_R16_SNORM,					// 58
	DXGI_FORMAT_R16_SINT,					// 59
	DXGI_FORMAT_R8_TYPELESS,				// 60
	DXGI_FORMAT_R8_UNORM,					// 61
	DXGI_FORMAT_R8_UINT,					// 62
	DXGI_FORMAT_R8_SNORM,					// 63
	DXGI_FORMAT_R8_SINT,					// 64
	DXGI_FORMAT_A8_UNORM,					// 65
	DXGI_FORMAT_R1_UNORM,					// 66	
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP,			// 67
	DXGI_FORMAT_R8G8_B8G8_UNORM,			// 68
	DXGI_FORMAT_G8R8_G8B8_UNORM,			// 69
	DXGI_FORMAT_BC1_TYPELESS,				// 70
	DXGI_FORMAT_BC1_UNORM,					// 71
	DXGI_FORMAT_BC1_UNORM_SRGB,				// 72
	DXGI_FORMAT_BC2_TYPELESS,				// 73
	DXGI_FORMAT_BC2_UNORM,					// 74
	DXGI_FORMAT_BC2_UNORM_SRGB,				// 75
	DXGI_FORMAT_BC3_TYPELESS,				// 76
	DXGI_FORMAT_BC3_UNORM,					// 77
	DXGI_FORMAT_BC3_UNORM_SRGB,				// 78
	DXGI_FORMAT_BC4_TYPELESS,				// 79
	DXGI_FORMAT_BC4_UNORM,					// 80
	DXGI_FORMAT_BC4_SNORM,					// 81
	DXGI_FORMAT_BC5_TYPELESS,				// 82
	DXGI_FORMAT_BC5_UNORM,					// 83
	DXGI_FORMAT_BC5_SNORM,					// 84
	DXGI_FORMAT_B5G6R5_UNORM,				// 85
	DXGI_FORMAT_B5G5R5A1_UNORM,				// 86
	DXGI_FORMAT_B8G8R8A8_UNORM,				// 87
	DXGI_FORMAT_B8G8R8X8_UNORM,				// 88
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,	// 89
	DXGI_FORMAT_B8G8R8A8_TYPELESS,			// 90
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,		// 91
	DXGI_FORMAT_B8G8R8X8_TYPELESS,			// 92
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,		// 93
	DXGI_FORMAT_BC6H_TYPELESS,				// 94
	DXGI_FORMAT_BC6H_UF16,					// 95
	DXGI_FORMAT_BC6H_SF16,					// 96
	DXGI_FORMAT_BC7_TYPELESS,				// 97
	DXGI_FORMAT_BC7_UNORM,					// 98
	DXGI_FORMAT_BC7_UNORM_SRGB,				// 99
	DXGI_FORMAT_AYUV,						// 100
	DXGI_FORMAT_Y410,						// 101
	DXGI_FORMAT_Y416,						// 102
	DXGI_FORMAT_NV12,						// 103
	DXGI_FORMAT_P010,						// 104
	DXGI_FORMAT_P016,						// 105
	DXGI_FORMAT_420_OPAQUE,					// 106
	DXGI_FORMAT_YUY2,						// 107
	DXGI_FORMAT_Y210,						// 108
	DXGI_FORMAT_Y216,						// 109
	DXGI_FORMAT_NV11,						// 110
	DXGI_FORMAT_AI44,						// 111
	DXGI_FORMAT_IA44,						// 112
	DXGI_FORMAT_P8,							// 113
	DXGI_FORMAT_A8P8,						// 114
	DXGI_FORMAT_B4G4R4A4_UNORM,				// 115
	DXGI_FORMAT_P208,						// 116
	DXGI_FORMAT_V208,						// 117
	DXGI_FORMAT_V408,						// 118
	DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,			// 119
	DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,	// 120
	DXGI_FORMAT_FORCE_UINT									// 121
} ;

enum D3D10_RESOURCE_MISC_FLAG 
{
	D3D10_RESOURCE_MISC_GENERATE_MIPS,
	D3D10_RESOURCE_MISC_SHARED,
	D3D10_RESOURCE_MISC_TEXTURECUBE,
	D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX,
	D3D10_RESOURCE_MISC_GDI_COMPATIBLE
} ;
#pragma pack (pop)

//	the following constants were copied directly off the MSDN website

//	The dwFlags member of the original DDSURFACEDESC2 structure
//	can be set to one or more of the following values.
#define DDSD_CAPS			0x00000001
#define DDSD_HEIGHT			0x00000002
#define DDSD_WIDTH			0x00000004
#define DDSD_PITCH			0x00000008
#define DDSD_PIXELFORMAT	0x00001000
#define DDSD_MIPMAPCOUNT	0x00020000
#define DDSD_LINEARSIZE		0x00080000
#define DDSD_DEPTH			0x00800000

//	DirectDraw Pixel Format
#define DDPF_ALPHAPIXELS	0x00000001
#define DDPF_FOURCC			0x00000004
#define DDPF_RGB			0x00000040

//	The dwCaps1 member of the DDSCAPS2 structure can be
//	set to one or more of the following values.
#define DDSCAPS_COMPLEX	0x00000008
#define DDSCAPS_TEXTURE	0x00001000
#define DDSCAPS_MIPMAP	0x00400000

//	The dwCaps2 member of the DDSCAPS2 structure can be
//	set to one or more of the following values.
#define DDSCAPS2_CUBEMAP			0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX	0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX	0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY	0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY	0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ	0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ	0x00008000
#define DDSCAPS2_VOLUME				0x00200000

/*
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
			(uint32_t)( \
			(((uint32_t)(uint8_t)(ch3) << 24) & 0xFF000000) | \
			(((uint32_t)(uint8_t)(ch2) << 16) & 0x00FF0000) | \
			(((uint32_t)(uint8_t)(ch1) <<  8) & 0x0000FF00) | \
			((uint32_t)(uint8_t)(ch0)        & 0x000000FF) )
*/

const uint32_t	BC4U = MAKEFOURCC('B','C','4','U');
const uint32_t	BC4S = MAKEFOURCC('B','C','4','S');
const uint32_t	DXT1 = 0x31545844;
const uint32_t	DXT3 = 0x33545844;
const uint32_t	DXT5 = 0x35545844;
const uint32_t	DX10 = 0x30315844;

static void uploadTextureData ( Device& device, Texture& texture, Buffer& stagingBuffer, VkFormat format, uint32_t arrayLayers, uint32_t mipLevels, int blockSize, int blockWidth = 1, int blockHeight = 1 )
{
	SingleTimeCommand				cmd ( device );
	std::vector<VkBufferImageCopy>	regions;
	auto							offset = 0;
	auto							w = texture.getWidth  ();
	auto							h = texture.getHeight ();
	auto							d = texture.getDepth  ();
	
	texture.getImage ().transitionLayout  ( cmd, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

	for ( uint32_t i = 0; i < mipLevels; i++ )
	{
		VkBufferImageCopy	region = {};

		region.bufferOffset                    = offset;
		region.bufferRowLength                 = 0;
		region.bufferImageHeight               = 0;
		region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel       = i;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount     = arrayLayers;
		region.imageOffset                     = {0, 0, 0};
		region.imageExtent                     = { w, h, d };

		offset += ((w + blockWidth - 1)/blockWidth) * ((h + blockHeight - 1)/blockHeight) * blockSize;

		regions.push_back ( region );

		if ( w > 1 )
			w /= 2;
		
		if ( h > 1 )
			h /= 2;
		
		if ( d > 1 )
			d /= 2;
	}

	vkCmdCopyBufferToImage ( cmd.getHandle (), stagingBuffer.getHandle (), texture.getImage ().getHandle (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, regions.data () );

	texture.getImage ().transitionLayout ( cmd, texture.getImage ().getFormat (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
}

void	loadDds ( Device& device, Texture& texture, Data&& data )
{
	if( sizeof( DdsHeader ) != 128 )
		fatal () << "DdsLoader: incorrect header size" << Log::endl;
	
	DdsHeader	hdr;
	int			offs = sizeof ( hdr );	// HEADER10 ??
	
	if ( data.getBytes ( &hdr, sizeof ( hdr ) ) != sizeof ( hdr ) )
		fatal () << "DdsLoader: error reading header" << Log::endl;
	
	if ( hdr.dwMagic != (('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24)) )
		fatal () << "Ddsloader: invalid magic" << Log::endl;
	
	if( hdr.dwSize != 124 ) 
		fatal () << "DdsLoader: invalid hdr.dwSize" << Log::endl;
	
	uint32_t	flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	
	if ( ( hdr.dwFlags & flags) != flags )
		fatal () << "DdsLoader: missing required flags" << Log::endl;
	
	/*	According to the MSDN spec, the dwFlags should contain
		DDSD_LINEARSIZE if it's compressed, or DDSD_PITCH if
		uncompressed.  Some DDS writers do not conform to the
		spec, so I need to make my reader more tolerant	*/
		
	if ( hdr.sPixelFormat.dwSize != 32 )
		fatal () << "DdsLoader: invalid sPixelFormat.dwSize" << Log::endl;
	
	flags = DDPF_FOURCC | DDPF_RGB;
	
	if ( (hdr.sPixelFormat.dwFlags & flags) == 0 )
		fatal () << "DdsLoader: missing required flags in sPixelFormat.dwFlags" << Log::endl;
	
	if ( (hdr.sCaps.dwCaps1 & DDSCAPS_TEXTURE) == 0 )
		fatal () << "DdsLoaderL missing required flags in sCaps.dwCaps1" << Log::endl;

		// get the image data
	uint32_t	width  = hdr.dwWidth;
	uint32_t	height = hdr.dwHeight;
	uint32_t	depth  = hdr.dwDepth;
	
	if ( (hdr.dwFlags & DDSD_DEPTH) == 0 )		// no depth
		depth = 1;

	bool	hasFourCC    = ((hdr.sPixelFormat.dwFlags & DDPF_FOURCC) / DDPF_FOURCC) != 0;				// just has dwFourCC
	bool	hasAlpha     = ((hdr.sPixelFormat.dwFlags & DDPF_ALPHAPIXELS) / DDPF_ALPHAPIXELS) != 0;
	bool	hasMipmap    = (hdr.sCaps.dwCaps1 & DDSCAPS_MIPMAP) && (hdr.dwMipMapCount > 1);
	bool	cubemapFaces = ((hdr.sCaps.dwCaps2 & DDSCAPS2_CUBEMAP) / DDSCAPS2_CUBEMAP) != 0;
	bool	isVolume     = (hdr.sCaps.dwCaps2 & DDSCAPS2_VOLUME) != 0;
	auto	mipLevels    = hdr.dwMipMapCount;
	auto	isCompressed = false;
	auto	format       = VK_FORMAT_R8G8B8A8_UNORM;
	auto	imageType    = VK_IMAGE_TYPE_2D;
	auto	texelSize    = 4;
	auto	blockSize    = 4;		// 4 bytes per block
	auto	blockWidth   = 1;		// for 1x1 block
	auto	blockHeight  = 1;
	auto	layerCount   = 1;		// 6 for cubemaps, may have other values for texture arrays
	auto	imageViewType = VK_IMAGE_VIEW_TYPE_2D;

	flags = 0;
	
	if ( cubemapFaces && ( width != height ) )
		fatal () << "DDsLoader: cubemap must have square faces" << Log::endl;

	if ( cubemapFaces )		// cubemap is just like array of 6 images
	{
		layerCount    = 6;
		imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
		flags         = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	if ( height == 1 )
	{
		imageType     = VK_IMAGE_TYPE_1D;
		imageViewType = VK_IMAGE_VIEW_TYPE_1D;
	}
	else
	if ( depth > 1 )
	{
		imageType     = VK_IMAGE_TYPE_3D;
		imageViewType = VK_IMAGE_VIEW_TYPE_3D;
	}

	if ( mipLevels < 1 )
		mipLevels = 1;

	if ( hasFourCC )
	{
		if ( hdr.sPixelFormat.dwFourCC == 0x30315844 )	// DX10 signature
		{
			Dx10Header	hdr10;

			offs += sizeof ( hdr10 );
			data.getBytes ( &hdr10, sizeof ( hdr10 ) );

			log () << "DXGI " << hdr10.dxgiFormat << " Resource dimension " << hdr10.resourceDimension << " arraySize " << hdr10.arraySize << hdr10.miscFlag << ", " << hdr10.miscFlags2 << Log::endl;

			if ( hdr10.dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM )
				format = VK_FORMAT_R8G8B8A8_UNORM;

			if ( hdr10.resourceDimension == D3D10_RESOURCE_DIMENSION_TEXTURE1D )
				imageType = VK_IMAGE_TYPE_1D;
			else
			if ( hdr10.resourceDimension == D3D10_RESOURCE_DIMENSION_TEXTURE2D )
				imageType = VK_IMAGE_TYPE_2D;
			else
			if ( hdr10.resourceDimension == D3D10_RESOURCE_DIMENSION_TEXTURE3D )
				imageType = VK_IMAGE_TYPE_3D;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == 36 )
		{
			format    = VK_FORMAT_R16G16B16A16_UNORM;
			texelSize = 8;
			blockSize = 8;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == 110 )	
		{
			format    = VK_FORMAT_R16G16B16A16_SNORM;
			texelSize = 8;
			blockSize = 8;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == 111 )		// GL_R16F
		{
			format    = VK_FORMAT_R16_SFLOAT;
			texelSize = 2;
			blockSize = 2;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == 112 )		// GL_RG16F
		{
			format    = VK_FORMAT_R16G16_SFLOAT;
			texelSize = 4;
			blockSize = 4;
		}
		else 
		if ( hdr.sPixelFormat.dwFourCC == 113 )		// GL_RGBA16F
		{
			format    = VK_FORMAT_R16G16B16A16_SFLOAT;
			texelSize = 8;
			blockSize = 8;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == 114 )		// GL_R32F
		{
			format    = VK_FORMAT_R32_SFLOAT;
			texelSize = 4;
			blockSize = 4;
		}
		else 
		if ( hdr.sPixelFormat.dwFourCC == 115 )		// GL_RG32F
		{
			format    = VK_FORMAT_R32G32_SFLOAT;
			texelSize = 8;
			blockSize = 8;
		}
		else 
		if ( hdr.sPixelFormat.dwFourCC == 116 )		// GL_RGBA32F
		{
			format    = VK_FORMAT_R32G32B32A32_SFLOAT;
			texelSize = 16;
			blockSize = 16;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == DXT1 )	// DXT1
		{
			isCompressed = true;
			blockSize    = 8;
			blockWidth   = 4;
			blockHeight  = 4;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == DXT3 )	// DXT3
		{
			isCompressed = true;
			blockSize    = 16;
			blockWidth   = 4;
			blockHeight  = 4;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == DXT5 )	// DXT5
		{
			isCompressed = true;
			blockSize    = 16;
			blockWidth   = 4;
			blockHeight  = 4;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == BC4U )	// BC4U
		{
			isCompressed = true;
			blockSize    = 16;
			blockWidth   = 4;
			blockHeight  = 4;
		}
		else
		if ( hdr.sPixelFormat.dwFourCC == BC4S )	// BC4S
		{
			isCompressed = true;
			blockSize    = 16;
			blockWidth   = 4;
			blockHeight  = 4;
		}
		
				// BC5 - 4x4 16 bytes
				// BC2 - 4x4 16 bytes
				// BC6 - 4x4 16 bytes
				// BC7 - 4x4 16 bytes
	}
	
	auto	w      = width, 
			h      = height, 
			d      = depth;
	auto	offset = 0;

	Buffer	stagingBuffer;
	
	stagingBuffer.create ( device, data.getLength () - offs, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
	stagingBuffer.copy   ( data.getPtr ( offs ), data.getLength () - offs );

	if ( isCompressed )
	{
		auto	blockPitch = (width + 3) >> 2;
		auto	numBlocks  = blockPitch * ((height + 3) >> 2);
		auto	dxtFamily  = 1 + (hdr.sPixelFormat.dwFourCC >> 24) - '1';
		//auto	blockSize  = dxtFamily == 1 ? 8 : 16;
		
		if ( (dxtFamily < 1) || (dxtFamily > 5) ) 
			fatal () << "DdsLoader: invalid DXT family " << dxtFamily << Log::endl;
		
		if ( dxtFamily == 1 )
			format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;		// ????
		else
		if ( dxtFamily == 3 )
			format = VK_FORMAT_BC2_UNORM_BLOCK;
		else
		if ( dxtFamily == 5 )
			format = VK_FORMAT_BC3_UNORM_BLOCK;

//		texture.getImage ().create ( device, width, height, depth, mipLevels, format, 
//									VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
//									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		texture.getImage ().create ( device, ImageCreateInfo ( width, width, depth )
											 .setFlags ( flags )
											 .setFormat ( format )
											 .setMipLevels ( mipLevels )
											 .setLayers ( layerCount )
											 .setUsage ( VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ) );

		texture.createImageView ( VK_IMAGE_ASPECT_COLOR_BIT, imageViewType );
	}
	else
	{
//			texture.getImage ().create ( device, width, height, depth, mipLevels, format, 
//				VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
//				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		texture.getImage ().create ( device, ImageCreateInfo ( width, width, depth )
											 .setFlags ( flags )
											 .setFormat ( format )
											 .setMipLevels ( mipLevels )
											 .setLayers ( layerCount )
											 .setUsage ( VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ) );

			texture.createImageView ( VK_IMAGE_ASPECT_COLOR_BIT, imageViewType );
	}

	uploadTextureData ( device, texture, stagingBuffer, format, layerCount, mipLevels, blockSize, blockWidth, blockHeight );

}

