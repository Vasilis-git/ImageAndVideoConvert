#include "../include/writers.h"

const Format img_formats[] = {
    {"png"},
    {"jpg"},
    {"jpeg"},
    {"jfif"},
    {"webp"},
    {"bmp"},
    {"tga"},
    {"pnm"},
};
const size_t img_formats_count = sizeof(img_formats) / sizeof(img_formats[0]);

int write_png(const char *path, const Image *img);
int write_jpg_wrapper(const char *path, const Image *img);
int write_webp_wrapper(const char *path, const Image *img);
int write_bmp(const char *path, const Image *img);
int write_tga(const char *path, const Image *img);
int write_pnm(const char *path, const Image *img);

const Writer writers[] = {
    {"png",  write_png},
    {"jpg",  write_jpg_wrapper},
    {"jpeg", write_jpg_wrapper},
    {"jfif", write_jpg_wrapper},
    {"webp", write_webp_wrapper},
    {"bmp",  write_bmp},
    {"tga",  write_tga},
    {"pnm",  write_pnm},
};
const size_t writers_count = sizeof(writers) / sizeof(writers[0]);