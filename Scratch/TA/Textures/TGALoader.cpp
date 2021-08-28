
#include "TGALoader.hpp"

//=========================================================================
//=========================================================================

struct Stream
{
    X_FILE* Fp;

    void Read( void* pBuff, s32 Count )
    {
        x_fread( pBuff, Count, 1, Fp );
    }

    void Write( void* pBuff, s32 Count )
    {
        x_fwrite( pBuff, Count, 1, Fp );
    }

    void Seek( s32 Offset, s32 Mode )
    {
        x_fseek( Fp, Offset, Mode );
    }
};

//=========================================================================

u16 ReadLittleEndian16(Stream& stream)
{
	u16 v;
	stream.Read(&v,sizeof(v));

#ifdef TARGET_GCN
	v = ENDIAN_SWAP_16( v );
#endif

    return v;
}

//=========================================================================

u8 ReadLittleEndian8(Stream& stream)
{
	u8 v;
	stream.Read(&v,sizeof(v));
    return v;
}

//=========================================================================

struct HeaderTGA
{
	u8		idfield_length;
	u8		colormap_type;
	u8		data_type;
	u16		colormap_origin;
	u16		colormap_length;
	u8		colormap_bitsize;
	u16		image_origin_x;
	u16		image_origin_y;
	u16		image_width;
	u16		image_height;
	u8		pixelsize;
	u8		descriptor;

	xbool ReadHeader( Stream& stream )
	{
		// read header
		idfield_length   = ReadLittleEndian8  (stream);
		colormap_type    = ReadLittleEndian8  (stream);
		data_type        = ReadLittleEndian8  (stream);
		colormap_origin  = ReadLittleEndian16 (stream);
		colormap_length  = ReadLittleEndian16 (stream);
		colormap_bitsize = ReadLittleEndian8  (stream);
		image_origin_x   = ReadLittleEndian16 (stream);
		image_origin_y   = ReadLittleEndian16 (stream);
		image_width      = ReadLittleEndian16 (stream);
		image_height     = ReadLittleEndian16 (stream);
		pixelsize        = ReadLittleEndian8  (stream);
		descriptor       = ReadLittleEndian8  (stream);

		// validate header
		switch ( data_type )
		{
			case 1:
			case 2:
			case 9:
			case 10: break;
			default: return FALSE;
		}

		switch ( pixelsize )
		{
			case 8:
			case 16:
			case 24:
			case 32: break;
			default: return FALSE;
		}

		if ( colormap_type > 1 )
			return FALSE;

		if ( (data_type == 1 || data_type == 9) && (colormap_bitsize != 24 || colormap_length > 256) )
			return FALSE;

		// everything seems to be in order
		return TRUE;
	}
};

//=========================================================================

static 
void UnpackLINEAR( Stream& stream, char* scan, s32 nextscan, s32 width, s32 height, s32 depth)
{
	s32 pitch = width * depth;
	for ( s32 y=0; y<height; ++y, scan+=nextscan )
	{
		// decode scanline
		stream.Read( scan, pitch );
	}
}

//=========================================================================

static 
void UnpackRLE(Stream& stream, char* scan, s32 nextscan, s32 width, s32 height, s32 depth)
{
	s32 x = 0;
	s32 y = 0;
	char* buffer = scan;

	for ( ;; )
	{
		u8 sample = ReadLittleEndian8(stream);
		s32 count = (sample & 0x7f) + 1;

		if ( sample & 0x80 )
		{
			u8 color[4];
			stream.Read(color,depth);

			for ( ; count; )
			{
				s32 left = width - x;
				s32 size = iMin(count,left);
				
				count -= size;
				x += size;
				
				for ( ; size; --size )
					for ( s32 j=0; j<depth; ++j )
						*buffer++ = color[j];
					
				if ( x >= width )
				{
					if ( ++y >= height )
						return;

					x = 0;
					scan += nextscan;
					buffer = scan;
				}
			}
		}
		else
		{
			for ( ; count; )
			{
				s32 left = width - x;
				s32 size = iMin(count,left);
				
				stream.Read(buffer,size*depth);
				buffer += size*depth;
				count -= size;

				x += size;
				if ( x >= width )
				{
					if ( ++y >= height )
						return;

					x = 0;
					scan += nextscan;
					buffer = scan;
				}
			}
		}
	}
}

//=========================================================================

static 
xbool tga_decode( xbitmap& target, Stream& stream )
{
	// reset stream
	stream.Seek( 0, X_SEEK_SET );

	// read header
	HeaderTGA header;
	if ( !header.ReadHeader(stream) )
		return FALSE;

	// skip idfield
	stream.Seek( header.idfield_length, X_SEEK_CUR );

	// information
    xbitmap::format pxf;
	switch ( header.pixelsize )
	{
        case 8:		pxf = xbitmap::FMT_P8_URGB_8888; break;
		case 16:	pxf = xbitmap::FMT_16_ARGB_1555; break;
		case 24:	pxf = xbitmap::FMT_24_RGB_888;   break;
		case 32:	pxf = xbitmap::FMT_32_ARGB_8888; break;
	}

	// read palette
    xcolor* palette = NULL;
	switch ( header.data_type )
	{
		case 2:
		case 10:
		{
			s32 delta = header.colormap_length * ((header.colormap_bitsize + 1) >> 3);
			stream.Seek( delta, X_SEEK_CUR );
			break;
		}

		case 1:
		case 9:
		{
			palette = new xcolor[256];
			for ( s32 i=0; i<static_cast<s32>(header.colormap_length); ++i )
			{
				palette[i].B = ReadLittleEndian8(stream);
				palette[i].G = ReadLittleEndian8(stream);
				palette[i].R = ReadLittleEndian8(stream);
				palette[i].A = 0xff;
			}
			break;
		}
	}

	// setup info
    const xbitmap::format_info& Info = xbitmap::GetFormatInfo( pxf );

	s32 width  = header.image_width;
	s32 height = header.image_height;
	s32 depth  = Info.BPP/8;
	s32 pitch  = width * depth;

	// set image
	char* image = new char[height * pitch];
	target.Setup( pxf, width, height, TRUE, (u8*)image, palette!=NULL, (u8*)palette, pitch/depth );

	// image orientation
	char* scan     = (header.descriptor & 32) ? image : image + (height - 1) * pitch;
	s32   nextscan = (header.descriptor & 32) ? pitch : -pitch;

	// decode image
	switch ( header.data_type )
	{
		case 1:  UnpackLINEAR(stream,scan,nextscan,width,height,depth); break;	// indexed 8bit (linear)
		case 2:	 UnpackLINEAR(stream,scan,nextscan,width,height,depth); break;	// rgb 16,24,32bit (linear)
		case 9:  UnpackRLE(stream,scan,nextscan,width,height,depth); break;		// indexed 8bit (rle)
		case 10: UnpackRLE(stream,scan,nextscan,width,height,depth); break;		// rgb 16,24,32bit (rle)
	}

	return TRUE;
}

//=========================================================================

void TGA_Load( xbitmap& Bitmap, const char* pFileName )
{
    Stream    stream;

    stream.Fp = x_fopen( pFileName, "rb" );
    if( stream.Fp == NULL ) return;

    Bitmap.Kill();

    tga_decode( Bitmap, stream );

    x_fclose( stream.Fp ); 
}