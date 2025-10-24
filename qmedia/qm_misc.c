#include "qmedia.h"
#include <png.h>
#include <zlib.h>
#include <jpeglib.h>
#include <jerror.h>
#include <turbojpeg.h>	// for transformation

#define PIXEL_SIZE		4	// pixel size in bytes
#define	NUM_CHANNELS	3	// RGB

void PNGtoMemory(png_structp png, png_bytep data, png_size_t length)
{
	compressed_image_t *write = png_get_io_ptr(png);
	png_size_t newsize = write->size + length;

	if (write->buffer)
	{
		write->buffer = realloc(write->buffer, newsize);
	}
	else write->buffer = malloc(newsize);

	if (!write->buffer)
		mi.Sys_Error(ERR_FATAL, "Out of memory\n");

	memcpy(write->buffer + write->size, data, length);
	write->size = newsize;
}

void PNGFlushData(png_structp png_ptr)
{
	// do nothing
}

compressed_image_t *QM_BitmapToPNG(byte *bitmap, int width, int height)
{
	png_structp png;
	png_infop info;
	png_byte **rows;
	compressed_image_t io, *compressed;
	int x, y;

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png)
		return NULL;

	info = png_create_info_struct(png);

	if (!info)
	{
		png_destroy_write_struct(&png, NULL);

		return NULL;
	}

	if (setjmp(png_jmpbuf(png)))
	{
		mi.Com_Printf("Error writing PNG\n");
		
		goto exit;
	}

	// set custom write function so png would be saved to memory
	io.buffer = NULL;
	io.size = 0;

	png_set_write_fn(png, &io, PNGtoMemory, PNGFlushData);

	// set up PNG data
	png_set_filter(png, 0, PNG_FILTER_NONE);
	png_set_compression_level(png, Z_BEST_COMPRESSION);
	png_set_compression_mem_level(png, 8);
	png_set_compression_strategy(png, Z_DEFAULT_STRATEGY);
	png_set_compression_window_bits(png, 15);
	png_set_compression_method(png, Z_DEFLATED);
	png_set_compression_buffer_size(png, 8192);

	// set image header
	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// input image data into structure
	png_write_info(png, info);

	// set up rows
	rows = png_malloc(png, sizeof(png_byte *) * height);
	
	if (rows == NULL)
		goto exit;

	for (y = 0; y < height; ++y)
	{
		png_byte *row = png_malloc(png, PIXEL_SIZE * width);
		rows[y] = row;

		if (row == NULL)
			goto exit;

		for (x = width; x > 0; --x)
		{
			*row++ = bitmap[y * width * 3 + x * 3];		// r
			*row++ = bitmap[y * width * 3 + x * 3 + 1];	// g
			*row++ = bitmap[y * width * 3 + x * 3 + 2];	// b
		}
	}

	// write the image
	png_write_image(png, rows);
	png_write_end(png, info);

	png_destroy_write_struct(&png, &info);

	compressed = malloc(sizeof(compressed_image_t));
	
	if (compressed)
	{
		compressed->buffer = io.buffer;
		compressed->size = io.size;

		return compressed;
	}

exit:
	png_destroy_write_struct(&png, &info);
	return NULL;
}

compressed_image_t *JPEGMirror(byte *buffer, long size)
{
	compressed_image_t* compressed;
	tjhandle handle = tjInitTransform();
	tjtransform tf;
	byte *dstbuf;
	long dstsize;

	compressed = malloc(sizeof(compressed_image_t));

	if (compressed == NULL)
		return NULL;

	memset(&tf, 0, sizeof(tjtransform));
	dstbuf = NULL;
	dstsize = 0;

	tf.op = TJXOP_HFLIP;
	tjTransform(handle, buffer, size, 1, &dstbuf, &dstsize, &tf, 0);

	tjDestroy(handle);

	compressed->buffer = dstbuf;
	compressed->size = dstsize;

	return compressed;
}

compressed_image_t *QM_BitmapToJPEG(byte *bitmap, int width, int height)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPARRAY image = NULL;
	JSAMPROW rowptr[1];
	int row_stride, row, col;

	byte *buffer;
	unsigned long *size;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	buffer = NULL;
	size = 0;

	jpeg_mem_dest(&cinfo, &buffer, &size);

	// fill in compress info
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = NUM_CHANNELS;
	cinfo.in_color_space = JCS_RGB;
	cinfo.data_precision = 8;

	// set quality
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 75, TRUE);
	cinfo.comp_info[0].h_samp_factor = cinfo.comp_info[0].v_samp_factor = 1;

	jpeg_start_compress(&cinfo, TRUE);

	row_stride = width * NUM_CHANNELS;

	// initialize image buffer
	image = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, height);

	for (row = 0; row < height; row++)
	{
		for (col = 0; col < width; col++)
		{
			image[row][col * 3] =
				bitmap[row * width * 3 + col * 3];
			
			image[row][col * 3 + 1] =
				bitmap[row * width * 3 + col * 3 + 1];
			
			image[row][col * 3 + 2] =
				bitmap[row * width * 3 + col * 3 + 2];
		}
	}

	// write scanlines
	while (cinfo.next_scanline < cinfo.image_height)
	{
		rowptr[0] = image[cinfo.next_scanline];
		jpeg_write_scanlines(&cinfo, rowptr, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	
	return JPEGMirror(buffer, size);
}