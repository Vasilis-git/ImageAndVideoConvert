#include "../include/tools.h"

static uint8_t *rgb_to_rgba(const Image *img) {
    size_t total = img->width * img->height;
    uint8_t *rgba = malloc(total * 4);
    if (!rgba) return NULL;

    for (size_t i = 0; i < total; i++) {
        rgba[i*4 + 0] = img->data[i*img->channels + 0];
        rgba[i*4 + 1] = img->data[i*img->channels + 1];
        rgba[i*4 + 2] = img->data[i*img->channels + 2];
        rgba[i*4 + 3] = 255;  // opaque alpha
    }

    return rgba;
}
static uint8_t *rgba_to_rgb(const Image *img) {
    size_t total = img->width * img->height;
    uint8_t *rgb = malloc(total * 3);
    if (!rgb) return NULL;

    for (size_t i = 0; i < total; i++) {
        rgb[i*3 + 0] = img->data[i*4 + 0];
        rgb[i*3 + 1] = img->data[i*4 + 1];
        rgb[i*3 + 2] = img->data[i*4 + 2];
    }

    return rgb;
}

int write_pnm(const char *path, const Image *img) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;

    size_t total_pixels = img->width * img->height;
    const uint8_t *data = img->data;
    int channels = img->channels;
    int free_after = 0;
    int format = 6;  // Default to PPM (P6)

    // Convert RGBA to RGB first if needed
    if (channels == 4) {
        data = rgba_to_rgb(img);
        if (!data) { fclose(f); return 0; }
        channels = 3;
        free_after = 1;
    }

    // Detect format: check if grayscale or binary
    int is_grayscale = (channels == 1);
    int is_binary = 1;

    if (!is_grayscale && channels == 3) {
        // Check if all RGB values are equal (grayscale)
        is_grayscale = 1;
        for (size_t i = 0; i < total_pixels; i++) {
            if (data[i*3] != data[i*3 + 1] || data[i*3] != data[i*3 + 2]) {
                is_grayscale = 0;
                break;
            }
        }
    }

    // Detect if binary (only 0 or 255 values)
    if (is_grayscale) {
        for (size_t i = 0; i < total_pixels; i++) {
            uint8_t val = (channels == 1) ? data[i] : data[i*3];
            if (val != 0 && val != 255) {
                is_binary = 0;
                break;
            }
        }
    } else {
        is_binary = 0;
    }

    // Determine PNM format
    if (is_binary) {
        format = 4;  // PBM (P4) - binary bitmap
    } else if (is_grayscale) {
        format = 5;  // PGM (P5) - grayscale
    } else {
        format = 6;  // PPM (P6) - color
    }

    // Write appropriate PNM header
    if (format == 4) {
        // PBM: binary bitmap
        fprintf(f, "P4\n%d %d\n", img->width, img->height);
    } else if (format == 5) {
        // PGM: grayscale
        fprintf(f, "P5\n%d %d\n255\n", img->width, img->height);
    } else {
        // PPM: color
        fprintf(f, "P6\n%d %d\n255\n", img->width, img->height);
    }

    // Write pixel data
    size_t bytes_to_write;
    if (format == 4) {
        // PBM: pack bits (8 pixels per byte)
        bytes_to_write = (img->width * img->height + 7) / 8;
        uint8_t *pbm_data = malloc(bytes_to_write);
        if (!pbm_data) { fclose(f); if (free_after) free((void*)data); return 0; }

        size_t bit_pos = 0;
        for (size_t i = 0; i < total_pixels; i++) {
            uint8_t val = (channels == 1) ? data[i] : data[i*3];
            uint8_t bit = (val > 127) ? 1 : 0;  // 1 for black, 0 for white in PBM
            size_t byte_idx = bit_pos / 8;
            size_t bit_idx = 7 - (bit_pos % 8);
            if (bit_idx == 7) pbm_data[byte_idx] = 0;
            pbm_data[byte_idx] |= (bit << bit_idx);
            bit_pos++;
        }
        fwrite(pbm_data, 1, bytes_to_write, f);
        free(pbm_data);
    } else if (format == 5) {
        // PGM: write grayscale data
        if (channels == 1) {
            bytes_to_write = total_pixels;
            fwrite(data, 1, bytes_to_write, f);
        } else {
            // Write only first channel (already verified all are equal)
            for (size_t i = 0; i < total_pixels; i++) {
                fputc(data[i*3], f);
            }
            bytes_to_write = total_pixels;
        }
    } else {
        // PPM: write RGB data
        bytes_to_write = total_pixels * 3;
        fwrite(data, 1, bytes_to_write, f);
    }

    fclose(f);
    if (free_after)
        free((void*)data);

    return 1;
}

int write_tga(const char *path, const Image *img) {
    return stbi_write_tga(
        path,
        img->width,
        img->height,
        img->channels,
        img->data
    );
}
int write_bmp(const char *path, const Image *img) {
    return stbi_write_bmp(
        path,
        img->width,
        img->height,
        img->channels,
        img->data
    );
}
int write_png(const char *path, const Image *img) {
    return stbi_write_png(
        path,
        img->width,
        img->height,
        img->channels,
        img->data,
        img->width * img->channels
    );
}

int write_jpg_wrapper(const char *path, const Image *img) {
    const uint8_t *data = img->data;
    int channels = img->channels;
    int free_after = 0;

    if (channels == 4) {
        data = rgba_to_rgb(img);
        if (!data) return 0;
        channels = 3;
        free_after = 1;
    }

    int ok = stbi_write_jpg(path, img->width, img->height, channels, data, 90);

    if (free_after)
        free((void*)data);

    return ok;
}

int write_webp_wrapper(const char *path, const Image *img) {
    float quality = 75.0f;

    uint8_t *data = img->data;
    int stride = img->width * img->channels;
    int free_after = 0;

    // WebPEncodeRGBA requires 4 channels
    if (img->channels == 3) {
        data = rgb_to_rgba(img);
        if (!data) return 0;
        stride = img->width * 4;
        free_after = 1;
    }

    uint8_t *out = NULL;
    size_t out_size = WebPEncodeRGBA(
        data,
        img->width,
        img->height,
        stride,
        quality,
        &out
    );

    if (free_after)
        free(data);

    if (out_size == 0 || !out)
        return 0;

    FILE *f = fopen(path, "wb");
    if (!f) {
        WebPFree(out);
        return 0;
    }

    fwrite(out, out_size, 1, f);
    fclose(f);
    WebPFree(out);
    return 1;
}
