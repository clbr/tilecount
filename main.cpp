#include <png.h>
#include <FL/fl_ask.H>

#include <algorithm>
#include <vector>

#include "common.h"

using namespace std;

static void diegui(const char msg[]) {
	fl_alert(msg);
	exit(1);
}

static void loadpng(const char name[], u8 **outdata, u32 *w, u32 *h) {

	FILE *f = fopen(name, "rb");
	if (!f)
		die("Can't open file\n");

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if (!png_ptr) die("PNG error\n");
	png_infop info = png_create_info_struct(png_ptr);
	if (!info) die("PNG error\n");
	if (setjmp(png_jmpbuf(png_ptr))) die("PNG error\n");

	png_init_io(png_ptr, f);
	png_read_png(png_ptr, info,
		PNG_TRANSFORM_PACKING|PNG_TRANSFORM_STRIP_16|PNG_TRANSFORM_STRIP_ALPHA|
		PNG_TRANSFORM_EXPAND|PNG_TRANSFORM_GRAY_TO_RGB,
		NULL);

	u8 **rows = png_get_rows(png_ptr, info);
	const u32 imgw = png_get_image_width(png_ptr, info);
	const u32 imgh = png_get_image_height(png_ptr, info);
	const u8 type = png_get_color_type(png_ptr, info);
	const u8 depth = png_get_bit_depth(png_ptr, info);

	if (imgw % 8 != 0 || imgh % 8 != 0)
		diegui("Error: Image is not divisible by 8!");

	if (type != PNG_COLOR_TYPE_RGB)
		die("Input must be a paletted PNG, got %u\n", type);

	if (depth != 8)
		die("Depth not 8 (%u) - maybe you have old libpng?\n", depth);

	const u32 rowbytes = png_get_rowbytes(png_ptr, info);
	if (rowbytes != imgw * 3)
		die("Packing failed, row was %u instead of %u\n", rowbytes, imgw * 3);

	u8 * const data = (u8 *) calloc(imgw * 3, imgh);
	u32 i;
	for (i = 0; i < imgh; i++) {
		u8 * const target = data + imgw * i * 3;
		memcpy(target, &rows[i][0], imgw * 3);
	}

	fclose(f);
	png_destroy_info_struct(png_ptr, &info);
	png_destroy_read_struct(&png_ptr, NULL, NULL);

	*outdata = data;
	*w = imgw;
	*h = imgh;
}

struct tile_t {
	u8 data[64*3];

	bool operator <(const tile_t &other) const {
		return memcmp(data, other.data, 64*3) < 0;
	}

	bool operator ==(const tile_t &other) const {
		return memcmp(data, other.data, 64*3) == 0;
	}
};

int main(int argc, char **argv) {

	u8 retval = 0;

	if (argc < 2) {
		die("Usage: %s image.png\n", argv[0]);
	}

	u8 *tilemap;
	u32 tilew, tileh, x, y;

	loadpng(argv[1], &tilemap, &tilew, &tileh);

	// Preprocess the tilemap into an easy-to-search format.
	const u32 numtiles = tilew * tileh / 64;
	vector<tile_t> tiles;
	tiles.resize(numtiles);

	u32 i;
	for (i = 0; i < numtiles; i++) {
		y = (i * 8) / tilew;
		x = (i * 8) % tilew;

		y *= 8;

		const u32 endx = x + 8;
		const u32 endy = y + 8;
		const u32 starty = y;

		u8 pix = 0;
		for (; x < endx; x++) {
			for (y = starty; y < endy; y++) {
				memcpy(tiles[i].data + pix * 3,
					tilemap + y * tilew * 3 + x * 3, 3);
				pix++;
			}
		}
		if (pix != 64) die("BUG, pix %u\n", pix);
	}

	sort(tiles.begin(), tiles.end());

	u32 sum = 1;
	for (i = 1; i < numtiles; i++) {
		if (!(tiles[i - 1] == tiles[i]))
			sum++;
	}

	fl_message("%u tiles.\n\n"
		"NES: aim for < 192.\n"
		"Genesis: aim for < 512.\n",
		sum);

	free(tilemap);
	return retval;
}
