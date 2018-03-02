/*
 * A simple libpng example program
 * http://zarb.org/~gc/html/libpng.html
 *
 * Modified by Yoshimasa Niwa to make it much simpler
 * and support all defined color_type.
 *
 * To build, use the next instruction on OS X.
 * $ brew install libpng
 * $ clang -lz -lpng15 libpng_test.c
 *
 * Copyright 2002-2010 Guillaume Cottenceau.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <png.h>

#include "util/Image.hh"
#include "String.hh"

namespace util {

bool Image::load(const String& filename) {
	FILE *fp = fopen(filename.c_str(), "rb");

	if (fp == nullptr)
		return false;

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png == nullptr)
		return false;

	png_infop info = png_create_info_struct(png);
	
	if (info == nullptr)
		return false;

	if (setjmp(png_jmpbuf(png)))
		return false;

	png_init_io(png, fp);
	png_read_info(png, info);

	width(png_get_image_width(png, info));
	height(png_get_image_height(png, info));
	png_byte color_type = png_get_color_type(png, info);
	png_byte bit_depth  = png_get_bit_depth(png, info);

	// Read any color_type into 8bit depth, RGBA format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	if(bit_depth == 16)
		png_set_strip_16(png);

	if(color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png);

	if(png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	// These color_type don't have an alpha channel then fill it with 0xff.
	if(color_type == PNG_COLOR_TYPE_RGB ||
		 color_type == PNG_COLOR_TYPE_GRAY ||
		 color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if(color_type == PNG_COLOR_TYPE_GRAY ||
		 color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);

	rows = new png_bytep [height()];

	for(unsigned int y = 0; y < height(); y++) {
		rows[y] = new png_byte [png_get_rowbytes(png,info)];
	}

	png_read_image(png, rows);
	png_destroy_read_struct(&png, &info, nullptr);

	fclose(fp);
	return true;
}

Image::
~Image() {
	if (rows != nullptr) {
		for (unsigned int h = 0; h < height(); h++)
			delete[] rows[h];

		delete[] rows;
	}
}

unsigned int Image::
value(Channel rgb, unsigned int x, unsigned int y) const {
	if (rows == nullptr || x > width() || y > height())
		return 0;

	return rows[y][x*4+rgb];
}

/*
void write_png_file(char *filename) {
	int y;

	FILE *fp = fopen(filename, "wb");
	if(!fp) abort();

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) abort();

	png_infop info = png_create_info_struct(png);
	if (!info) abort();

	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	// Output is 8bit depth, RGBA format.
	png_set_IHDR(
		png,
		info,
		width, height,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);

	// To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
	// Use png_set_filler().
	//png_set_filler(png, 0, PNG_FILLER_AFTER);

	png_write_image(png, row_pointers);
	png_write_end(png, NULL);

	for(int y = 0; y < height; y++) {
		free(row_pointers[y]);
	}
	free(row_pointers);

	fclose(fp);
}

void process_png_file() {
	for(int y = 0; y < height; y++) {
		png_bytep row = row_pointers[y];
		for(int x = 0; x < width; x++) {
			png_bytep px = &(row[x * 4]);
			// Do something awesome for each pixel here...
			//printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);
		}
	}
}

int main(int argc, char *argv[]) {
	if(argc != 3) abort();

	read_png_file(argv[1]);
	process_png_file();
	write_png_file(argv[2]);

	return 0;
}
*/

} // namespace util
