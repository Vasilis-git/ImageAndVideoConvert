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
