#ifndef SORTS_H
#define SORTS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "stb_image.h"
#include "stb_image_write.h"
#include <webp/encode.h>
#include <webp/decode.h>


typedef struct {
    char* input_file_name;
    char* output_format;
} CmdArgs;

typedef struct {
    const char *name;
} Format;

typedef struct {
    int width;
    int height;
    int channels;      // 1 = gray, 3 = RGB, 4 = RGBA
    unsigned char *data;
    int webp_allocated; // set to 1 when data was allocated by libwebp and must be freed with WebPFree
} Image;

typedef int (*WriteFunc)(const char*, const Image*);

typedef struct {
    const char *ext;
    WriteFunc fn;
} Writer;

CmdArgs parse_cmd(int argc, char* argv[]);
void make_output_filename(const char *input, const char *fmt, char *out, size_t outsz);
int img_convert(const char *input_path, const char *output_path);

#endif /* SORTS_H */