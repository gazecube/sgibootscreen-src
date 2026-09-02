#include "bootscreen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXT ".bmp"

static unsigned int read_le32(const unsigned char *p)
{
    return ((unsigned int)p[0]) |
           ((unsigned int)p[1] << 8) |
           ((unsigned int)p[2] << 16) |
           ((unsigned int)p[3] << 24);
}

static void write_le32(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v);
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

/*
 * Expand a 24-bit BMP horizontally without scaling its contents.
 * The original image is centered, and the new pixels inherit each
 * scanline's far-right background color.  Since the SGI background is
 * a horizontal solid-color gradient per row, this extends it seamlessly.
 *
 * 1280 -> 1820 adds 270 pixels to each side.  A centered 4:3 crop of
 * the result therefore recovers the original 1280x1024 composition.
 */
static int expand_bmp_width_centered(const char *filename, unsigned int new_width)
{
    const char *tmpname = "indy.wide.tmp.bmp";
    FILE *in = fopen(filename, "rb");
    if (!in)
        return -1;

    unsigned char fixed[54];
    if (fread(fixed, 1, sizeof(fixed), in) != sizeof(fixed) ||
        fixed[0] != 'B' || fixed[1] != 'M') {
        fclose(in);
        return -1;
    }

    unsigned int data_offset = read_le32(&fixed[10]);
    unsigned int old_width = read_le32(&fixed[18]);
    unsigned int height = read_le32(&fixed[22]);
    unsigned int bpp = (unsigned int)fixed[28] | ((unsigned int)fixed[29] << 8);
    unsigned int compression = read_le32(&fixed[30]);

    if (bpp != 24 || compression != 0 || new_width <= old_width || data_offset < 54) {
        fclose(in);
        return -1;
    }

    unsigned int left_pad = (new_width - old_width) / 2;
    unsigned int old_stride = (old_width * 3u + 3u) & ~3u;
    unsigned int new_stride = (new_width * 3u + 3u) & ~3u;
    unsigned int image_size = new_stride * height;
    unsigned int file_size = data_offset + image_size;

    unsigned char *header = malloc(data_offset);
    unsigned char *oldrow = malloc(old_stride);
    unsigned char *newrow = malloc(new_stride);
    if (!header || !oldrow || !newrow) {
        free(header); free(oldrow); free(newrow);
        fclose(in);
        return -1;
    }

    rewind(in);
    if (fread(header, 1, data_offset, in) != data_offset) {
        free(header); free(oldrow); free(newrow);
        fclose(in);
        return -1;
    }

    write_le32(&header[2], file_size);
    write_le32(&header[18], new_width);
    write_le32(&header[34], image_size);

    FILE *out = fopen(tmpname, "wb");
    if (!out) {
        free(header); free(oldrow); free(newrow);
        fclose(in);
        return -1;
    }

    fwrite(header, 1, data_offset, out);
    for (unsigned int y = 0; y < height; ++y) {
        if (fread(oldrow, 1, old_stride, in) != old_stride)
            break;

        const unsigned char *bg = &oldrow[(old_width - 1u) * 3u];
        for (unsigned int x = 0; x < new_width; ++x) {
            newrow[x * 3u + 0] = bg[0];
            newrow[x * 3u + 1] = bg[1];
            newrow[x * 3u + 2] = bg[2];
        }
        memset(newrow + new_width * 3u, 0, new_stride - new_width * 3u);
        memcpy(newrow + left_pad * 3u, oldrow, old_width * 3u);
        fwrite(newrow, 1, new_stride, out);
    }

    fclose(out);
    fclose(in);
    free(header); free(oldrow); free(newrow);

    remove(filename);
    return rename(tmpname, filename);
}

/* Never got around to adding any arguments for drawing
 * just the systems you care about, but doesn't really matter.
 */
int main(int argc, char **argv)
{
	draw_bootscreen(VGX, "vgx"EXT);
	draw_bootscreen(MG1_ENTRY, "mg1_entry"EXT);
	draw_bootscreen(MG1_EXPRESS, "mg1_express"EXT);
	draw_bootscreen(REALITY_ENGINE, "reality_engine"EXT);
	draw_bootscreen(REALITY_ENGINE_NEW, "reality_engine_new"EXT);
	draw_bootscreen(INDIGO, "indigo"EXT);
	draw_bootscreen(INDIGO2, "indigo2"EXT);
	draw_bootscreen(INDY, "indy"EXT);
	expand_bmp_width_centered("indy"EXT, 1820);
	draw_bootscreen(POWERINDIGO2, "powerindigo2"EXT);
	draw_bootscreen(INDIGO2R10K, "indigo2r10k"EXT);
	draw_bootscreen(OCTANE, "octane"EXT);
	draw_bootscreen(O2, "o2"EXT);
	draw_bootscreen(FUEL, "fuel"EXT);
	draw_bootscreen(INFINITE_PERFORMANCE, "infinite_performance"EXT);
	draw_bootscreen(INFINITE_REALITY, "infinite_reality"EXT);
	draw_bootscreen(TEZRO, "tezro"EXT);
	draw_bootscreen(ULTIMATE_VISION, "ultimate_vision"EXT);

	return 0;
}
