#ifndef __TGA_IMAGE__
#define __TGA_IMAGE__

#include	<cstdint>

#pragma pack (push, 1)
                                        // TGA specific constants and structs
struct  TgaHeader
{
    uint8_t   idLength;
    uint8_t   colormapType;
    uint8_t   imageType;
    uint16_t  colormapIndex;
    uint16_t  colormapLength;
    uint8_t   colormapEntrySize;
    uint16_t  xOrigin;
    uint16_t  yOrigin;
    uint16_t  width;
    uint16_t  height;
    uint8_t   pixelSize;
    uint8_t   imageDesc;
};

#pragma pack (pop)

class TgaImage 
{
protected:
	uint8_t * data;
	uint32_t	width;
	uint32_t	height;

public:
	TgaImage  ( uint32_t w, uint32_t h );
	~TgaImage ();

	uint32_t getWidth () const
	{
		return width;
	}

	uint32_t getHeight () const
	{
		return height;
	}

	uint8_t * getData ()
	{
		return data;
	}
	
	bool writeToFile ( const char * filename );
	
	TgaImage&	clear       ();
	TgaImage&	setRgbData  ( const uint8_t * ptr );
	TgaImage&	setRgbaData ( const uint8_t * ptr, bool swap = true );

	uint32_t getPixel ( uint32_t x, uint32_t y ) const
	{
		auto	pos = ((height-1-y) * width + x) * 3;
		
		return (uint32_t) data [pos+2] + (((uint32_t)data [pos+1]) << 8) + (((uint32_t)data [pos]) << 16);
	}

	void     putPixel ( uint32_t x, uint32_t y, uint32_t color )
	{
		auto	pos = ((height-1-y) * width + x) * 3;
		
		data [pos+2] = (uint8_t)(color & 0xFF);	
		data [pos+1] = (uint8_t)((color >> 8) & 0xFF);
		data [pos]   = (uint8_t)((color >> 16) & 0xFF);
	}
	
	uint32_t	rgbToInt ( uint32_t red, uint32_t green, uint32_t blue ) const
	{
		return red | (green << 8) | (blue << 16);
	}
};

#endif
