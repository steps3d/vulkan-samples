#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include	"Log.h"
#include	"Device.h"
#include	"Texture.h"

#pragma once

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

#pragma pack (pop)

//	the following constants were copied directly off the MSDN website

//	The dwFlags member of the original DDSURFACEDESC2 structure
//	can be set to one or more of the following values.
#define DDSD_CAPS	0x00000001
#define DDSD_HEIGHT	0x00000002
#define DDSD_WIDTH	0x00000004
#define DDSD_PITCH	0x00000008
#define DDSD_PIXELFORMAT	0x00001000
#define DDSD_MIPMAPCOUNT	0x00020000
#define DDSD_LINEARSIZE	0x00080000
#define DDSD_DEPTH	0x00800000

//	DirectDraw Pixel Format
#define DDPF_ALPHAPIXELS	0x00000001
#define DDPF_FOURCC		0x00000004
#define DDPF_RGB		0x00000040

//	The dwCaps1 member of the DDSCAPS2 structure can be
//	set to one or more of the following values.
#define DDSCAPS_COMPLEX	0x00000008
#define DDSCAPS_TEXTURE	0x00001000
#define DDSCAPS_MIPMAP	0x00400000

//	The dwCaps2 member of the DDSCAPS2 structure can be
//	set to one or more of the following values.
#define DDSCAPS2_CUBEMAP		0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX	0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX	0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY	0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY	0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ	0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ	0x00008000
#define DDSCAPS2_VOLUME			0x00200000

void	loadDds ( Texture& texture, Data& data )
{
	if( sizeof( DDS_header ) != 128 )
		fatal () << "DdsLoader: incorrect header size" << Log::endl;
	
	DdsHeader	hdr;
	
	if ( data.getBytes ( &hdr, sizeof ( hdr ) != sizeof ( hdr ) )
		fatal () << "DdsLoader: error reading header" << Log::endl;
	
	if ( hdr.dwMagic != (('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24)) )
		fatal () << "Ddsloader: invalid magic" << Log::endl;
	
	if( hdr.dwSize != 124 ) 
		fatal () << "DdsLoader: invalid hdr.dwSize" << Log::endl;
	
	uint32_t	DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	
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
	
	//s->img_n = 4;
	bool	isCompressed = ((hdr.sPixelFormat.dwFlags & DDPF_FOURCC) / DDPF_FOURCC) != 0;
	bool	hasAlpha     = ((hdr.sPixelFormat.dwFlags & DDPF_ALPHAPIXELS) / DDPF_ALPHAPIXELS) != 0;
	bool	hasMipmap    = ((hdr.sCaps.dwCaps1 & DDSCAPS_MIPMAP) && (header.dwMipMapCount > 1);
	bool	cubemapFaces = ((hdr.sCaps.dwCaps2 & DDSCAPS2_CUBEMAP) / DDSCAPS2_CUBEMAP) != 0;
	bool	isVolume     = ((hdr.sCaps.dwCaps2 & DDSCAPS2_VOLUME) != 0;
	
	if ( cubemapFaces && ( width != height ) )
		fatal () << "DDsLoader: cubemap must have square faces" << Log::endl;
	
	auto	blockPitch = (width + 3) >> 2;
	auto	numBlocks  = blockPitch * ((heught + 3) >> 2);
	
	if ( compressed )
	{
		auto	dxtFamily = 1 + (hdr.sPixelFormat.dwFourCC >> 24) - '1';
		
		if ( (dxtFamily < 1) || (dxtFamily > 5) ) 
			fatal () << "DdsLoader: invalid DXT family " << dxtFamily << Log::endl;
		
		

		
	}
	else
	{
		if ( isVolume )		// we're creating 3D texture
		{


		}
		else
		if ( cubemapFaces )	// we're creating a cubemap
		{

		}
		else			// it's a regular 1D/2D texture
		{


		}
	}
	
}

static stbi_uc *dds_load(stbi *s, int *x, int *y, int *comp, int req_comp)
{
	//	all variables go up front
	stbi_uc *dds_data = NULL;
	stbi_uc block[16*4];
	stbi_uc compressed[8];
	int flags, DXT_family;
	int has_alpha, has_mipmap;
	int is_compressed, cubemap_faces;
	int block_pitch, num_blocks;
	DDS_header header;
	int i, sz, cf;
	//	load the header
	if( sizeof( DDS_header ) != 128 )
	{
		return NULL;
	}
	getn( s, (stbi_uc*)(&header), 128 );
	//	and do some checking
	if( header.dwMagic != (('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24)) ) return NULL;
	if( header.dwSize != 124 ) return NULL;
	flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	if( (header.dwFlags & flags) != flags ) return NULL;
	/*	According to the MSDN spec, the dwFlags should contain
		DDSD_LINEARSIZE if it's compressed, or DDSD_PITCH if
		uncompressed.  Some DDS writers do not conform to the
		spec, so I need to make my reader more tolerant	*/
	if( header.sPixelFormat.dwSize != 32 ) return NULL;
	flags = DDPF_FOURCC | DDPF_RGB;
	if( (header.sPixelFormat.dwFlags & flags) == 0 ) return NULL;
	if( (header.sCaps.dwCaps1 & DDSCAPS_TEXTURE) == 0 ) return NULL;
	//	get the image data
	s->img_x = header.dwWidth;
	s->img_y = header.dwHeight;
	s->img_n = 4;
	is_compressed = (header.sPixelFormat.dwFlags & DDPF_FOURCC) / DDPF_FOURCC;
	has_alpha = (header.sPixelFormat.dwFlags & DDPF_ALPHAPIXELS) / DDPF_ALPHAPIXELS;
	has_mipmap = (header.sCaps.dwCaps1 & DDSCAPS_MIPMAP) && (header.dwMipMapCount > 1);
	cubemap_faces = (header.sCaps.dwCaps2 & DDSCAPS2_CUBEMAP) / DDSCAPS2_CUBEMAP;
	/*	I need cubemaps to have square faces	*/
	cubemap_faces &= (s->img_x == s->img_y);
	cubemap_faces *= 5;
	cubemap_faces += 1;
	block_pitch = (s->img_x+3) >> 2;
	num_blocks = block_pitch * ((s->img_y+3) >> 2);
	/*	let the user know what's going on	*/
	*x = s->img_x;
	*y = s->img_y;
	*comp = s->img_n;
	/*	is this uncompressed?	*/
	if( is_compressed )
	{
		/*	compressed	*/
		//	note: header.sPixelFormat.dwFourCC is something like (('D'<<0)|('X'<<8)|('T'<<16)|('1'<<24))
		DXT_family = 1 + (header.sPixelFormat.dwFourCC >> 24) - '1';
		if( (DXT_family < 1) || (DXT_family > 5) ) return NULL;
		/*	check the expected size...oops, nevermind...
			those non-compliant writers leave
			dwPitchOrLinearSize == 0	*/
		//	passed all the tests, get the RAM for decoding
		sz = (s->img_x)*(s->img_y)*4*cubemap_faces;
		dds_data = (unsigned char*)malloc( sz );
		/*	do this once for each face	*/
		for( cf = 0; cf < cubemap_faces; ++ cf )
		{
			//	now read and decode all the blocks
			for( i = 0; i < num_blocks; ++i )
			{
				//	where are we?
				int bx, by, bw=4, bh=4;
				int ref_x = 4 * (i % block_pitch);
				int ref_y = 4 * (i / block_pitch);
				//	get the next block's worth of compressed data, and decompress it
				if( DXT_family == 1 )
				{
					//	DXT1
					getn( s, compressed, 8 );
					stbi_decode_DXT1_block( block, compressed );
				} else if( DXT_family < 4 )
				{
					//	DXT2/3
					getn( s, compressed, 8 );
					stbi_decode_DXT23_alpha_block ( block, compressed );
					getn( s, compressed, 8 );
					stbi_decode_DXT_color_block ( block, compressed );
				} else
				{
					//	DXT4/5
					getn( s, compressed, 8 );
					stbi_decode_DXT45_alpha_block ( block, compressed );
					getn( s, compressed, 8 );
					stbi_decode_DXT_color_block ( block, compressed );
				}
				//	is this a partial block?
				if( ref_x + 4 > s->img_x )
				{
					bw = s->img_x - ref_x;
				}
				if( ref_y + 4 > s->img_y )
				{
					bh = s->img_y - ref_y;
				}
				//	now drop our decompressed data into the buffer
				for( by = 0; by < bh; ++by )
				{
					int idx = 4*((ref_y+by+cf*s->img_x)*s->img_x + ref_x);
					for( bx = 0; bx < bw*4; ++bx )
					{

						dds_data[idx+bx] = block[by*16+bx];
					}
				}
			}
			/*	done reading and decoding the main image...
				skip MIPmaps if present	*/
			if( has_mipmap )
			{
				int block_size = 16;
				if( DXT_family == 1 )
				{
					block_size = 8;
				}
				for( i = 1; i < header.dwMipMapCount; ++i )
				{
					int mx = s->img_x >> (i + 2);
					int my = s->img_y >> (i + 2);
					if( mx < 1 )
					{
						mx = 1;
					}
					if( my < 1 )
					{
						my = 1;
					}
					skip( s, mx*my*block_size );
				}
			}
		}/* per cubemap face */
	} else
	{
		/*	uncompressed	*/
		DXT_family = 0;
		s->img_n = 3;
		if( has_alpha )
		{
			s->img_n = 4;
		}
		*comp = s->img_n;
		sz = s->img_x*s->img_y*s->img_n*cubemap_faces;
		dds_data = (unsigned char*)malloc( sz );
		/*	do this once for each face	*/
		for( cf = 0; cf < cubemap_faces; ++ cf )
		{
			/*	read the main image for this face	*/
			getn( s, &dds_data[cf*s->img_x*s->img_y*s->img_n], s->img_x*s->img_y*s->img_n );
			/*	done reading and decoding the main image...
				skip MIPmaps if present	*/
			if( has_mipmap )
			{
				for( i = 1; i < header.dwMipMapCount; ++i )
				{
					int mx = s->img_x >> i;
					int my = s->img_y >> i;
					if( mx < 1 )
					{
						mx = 1;
					}
					if( my < 1 )
					{
						my = 1;
					}
					skip( s, mx*my*s->img_n );
				}
			}
		}
		/*	data was BGR, I need it RGB	*/
		for( i = 0; i < sz; i += s->img_n )
		{
			unsigned char temp = dds_data[i];
			dds_data[i] = dds_data[i+2];
			dds_data[i+2] = temp;
		}
	}
	/*	finished decompressing into RGBA,
		adjust the y size if we have a cubemap
		note: sz is already up to date	*/
	s->img_y *= cubemap_faces;
	*y = s->img_y;
	//	did the user want something else, or
	//	see if all the alpha values are 255 (i.e. no transparency)
	has_alpha = 0;
	if( s->img_n == 4)
	{
		for( i = 3; (i < sz) && (has_alpha == 0); i += 4 )
		{
			has_alpha |= (dds_data[i] < 255);
		}
	}
	if( (req_comp <= 4) && (req_comp >= 1) )
	{
		//	user has some requirements, meet them
		if( req_comp != s->img_n )
		{
			dds_data = convert_format( dds_data, s->img_n, req_comp, s->img_x, s->img_y );
			*comp = s->img_n;
		}
	} else
	{
		//	user had no requirements, only drop to RGB is no alpha
		if( (has_alpha == 0) && (s->img_n == 4) )
		{
			dds_data = convert_format( dds_data, 4, 3, s->img_x, s->img_y );
			*comp = 3;
		}
	}
	//	OK, done
	return dds_data;
}

bool	Texture :: load2DHdr ( const std::string& file )
{
//    stbi_set_flip_vertically_on_load ( true );

	fileName = file;
	
	int 	numComponents;
	float * data = stbi_loadf ( fileName.c_str (), &width, &height, &numComponents, 0 );
	
	if ( data == nullptr )
	{
			// check for floating-point DDS 
		Data	     data ( fileName );
		if ( (size_t)data.getLength () < sizeof ( DDS_header ) )
			return false;

		DDS_header * hdr = (DDS_header *) data.getPtr ();

		if ( hdr->dwMagic != (('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24)) ) 
			return false;
		
		if ( hdr->dwSize != 124 ) 
			return false;

		unsigned	flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

		if ( (hdr->dwFlags & flags) != flags ) 
			return false;

		if ( hdr->sPixelFormat.dwSize != 32 ) 
			return false;

		flags = DDPF_FOURCC | DDPF_RGB;

		if ( (hdr->sPixelFormat.dwFlags & flags) == 0 ) 
			return false;

		if ( (hdr->sCaps.dwCaps1 & DDSCAPS_TEXTURE) == 0 ) 
			return false;

		if ( (hdr->sCaps.dwCaps2 & DDSCAPS2_CUBEMAP) != 0 )
			return false;
		
		target = GL_TEXTURE_2D;
		width  = hdr->dwWidth;
		height = hdr->dwHeight;
		depth  = 1;
	
		if ( hdr->sPixelFormat.dwFourCC == 113 )	// GL_RGBA16F
		{
			glGenTextures ( 1, &id );
			glBindTexture ( target, id );
			glTexImage2D  ( GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, hdr+1 );
		
			goto ok;
		}
		else if ( hdr->sPixelFormat.dwFourCC == 111 )	// GL_R16F
		{
			glGenTextures ( 1, &id );
			glBindTexture ( target, id );
			glTexImage2D  ( GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_HALF_FLOAT, hdr+1 );
		
			goto ok;
		}
		else if ( hdr->sPixelFormat.dwFourCC == 112 )	// GL_RG16F
		{
			glGenTextures ( 1, &id );
			glBindTexture ( target, id );
			glTexImage2D  ( GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_HALF_FLOAT, hdr+1 );
		
			goto ok;
		}
		else if ( hdr->sPixelFormat.dwFourCC == 114 )	// GL_R32F
		{
			glGenTextures ( 1, &id );
			glBindTexture ( target, id );
			glTexImage2D  ( GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, hdr+1 );
		
			goto ok;
		}
		else if ( hdr->sPixelFormat.dwFourCC == 115 )	// GL_RG32F
		{
			glGenTextures ( 1, &id );
			glBindTexture ( target, id );
			glTexImage2D  ( GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, hdr+1 );
		
			goto ok;
		}
		else if ( hdr->sPixelFormat.dwFourCC == 116 )	// GL_RGBA32F
		{
			glGenTextures ( 1, &id );
			glBindTexture ( target, id );
			glTexImage2D  ( GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, hdr+1 );
		
			goto ok;
		}

		return false;
	}
	
	target = GL_TEXTURE_2D;
	depth  = 1;
	
	glGenTextures ( 1, &id );
	glBindTexture ( target, id );
	glTexImage2D  ( GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data );

	stbi_image_free ( data );
ok:;

	glTexParameteri ( target, GL_TEXTURE_WRAP_S, GL_REPEAT );    // set default params for texture
	glTexParameteri ( target, GL_TEXTURE_WRAP_T, GL_REPEAT );
	
	if ( autoMipmaps )
		glGenerateMipmap ( target );
	
	if ( autoMipmaps )
	{
		glTexParameteri ( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri ( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	else
	{
		glTexParameteri ( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri ( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	
	glBindTexture  ( target, 0 );

	return true;
}

bool	Texture :: loadCubemapHdr ( const std::string& file )
{
	fileName = file;
	
			// check for floating-point DDS 
	Data	     data ( fileName );
	
	if ( (size_t)data.getLength () < sizeof ( DDS_header ) )
		return false;

	DDS_header * hdr = (DDS_header *) data.getPtr ();

	if ( hdr->dwMagic != (('D' << 0) | ('D' << 8) | ('S' << 16) | (' ' << 24)) ) 
		return false;
	
	if ( hdr->dwSize != 124 ) 
		return false;

	unsigned	flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

	if ( (hdr->dwFlags & flags) != flags ) 
		return false;

	if ( hdr->sPixelFormat.dwSize != 32 ) 
		return false;

	flags = DDPF_FOURCC | DDPF_RGB;

	if ( (hdr->sPixelFormat.dwFlags & flags) == 0 ) 
		return false;

	if ( (hdr->sCaps.dwCaps1 & DDSCAPS_TEXTURE) == 0 ) 
		return false;

	if ( (hdr->sCaps.dwCaps2 & DDSCAPS2_CUBEMAP) == 0 )
		return false;
	
	uint8_t * ptr = (uint8_t *)(hdr + 1);		// pointer to raw image data
	
	if ( hdr->sPixelFormat.dwFourCC == 113 )	// GL_RGBA16F
	{
		target = GL_TEXTURE_CUBE_MAP;
		width  = hdr->dwWidth;
		height = hdr->dwHeight;
		depth  = 1;

		glGenTextures ( 1, &id );
		glBindTexture ( target, id );
		
		for ( int i = 0; i < 6; i++ )
			glTexImage2D  ( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, ptr + i * 2 * width * height );
	}

	glTexParameterf ( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf ( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri ( target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	
	if ( autoMipmaps )
		glGenerateMipmap ( target );
	
	if ( autoMipmaps )
	{
		glTexParameteri ( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri ( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	else
	{
		glTexParameteri ( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri ( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	
	glBindTexture  ( target, 0 );

	return true;
}
