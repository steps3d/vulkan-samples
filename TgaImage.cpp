#include    <memory.h>
#ifdef	_WIN32
    #include	<fcntl.h>
    #include	<io.h>
    #include	<sys/stat.h>
#else
    #include	<unistd.h>
    #include	<sys/types.h>
    #include	<sys/stat.h>
    #include	<fcntl.h>
    #define	O_BINARY	0
#endif

#include "TgaImage.h"

TgaImage::TgaImage ( uint32_t w, uint32_t h ) : data(nullptr), width(w), height(h)
{
	auto	numBytes = width*height*3;

	data = new uint8_t [numBytes];

	memset ( data, 0, numBytes);
}

TgaImage::~TgaImage () 
{
	delete [] data;
}

bool TgaImage::writeToFile ( const char * fileName )
{
	int	file = open ( fileName, O_RDWR | O_BINARY | O_CREAT, S_IWRITE | S_IREAD );
	
	if ( file == -1 )
		return false;
	
	TgaHeader	header;
	int			numBytes = width * height * 3;
	
	memset ( &header, '\0', sizeof ( header ) );
	
	header.imageType = 2;              // unencoded image
	header.width     = width;
	header.height    = height;
	header.pixelSize = 24;
	
	write ( file, &header, sizeof ( header ) );
	write ( file, data, numBytes );
	close ( file );
	
	return true;
}

TgaImage&     TgaImage::clear ()
{
	memset ( data, '\0', width * height * 3 );

	return *this;
}

TgaImage& 	TgaImage :: setRgbData ( const uint8_t * ptr )
{
	int	numPixels = width * height;
	int	offs      = 0;
	
	for ( int i = 0; i < numPixels; i++, offs += 3 )
	{
		data [offs]   = ptr [offs+2];
		data [offs+1] = ptr [offs+1];
		data [offs+2] = ptr [offs];
	}

	return *this;
}

TgaImage& 	TgaImage :: setRgbaData ( const uint8_t * ptr, bool swap )
{
	int	numPixels = width * height;
	int	offs      = 0;

	if ( !swap )
		for ( int i = 0; i < numPixels; i++, offs += 3, ptr += 4 )
		{
			data [offs]   = ptr [2];
			data [offs+1] = ptr [1];
			data [offs+2] = ptr [0];
		}
	else
		for ( int i = 0; i < numPixels; i++, offs += 3, ptr += 4 )
		{
			data [offs]   = ptr [0];
			data [offs+1] = ptr [1];
			data [offs+2] = ptr [2];
		}

	return *this;
}
