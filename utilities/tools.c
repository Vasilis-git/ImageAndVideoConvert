#include "../include/tools.h"

const char *match_format(const char *s) {
    for (size_t i = 0; i < sizeof(img_formats)/sizeof(img_formats[0]); i++) {
        if (strcmp(s, img_formats[i].name) == 0)
            return img_formats[i].name;
    }
    return NULL;
}
void list_valid_img_formats() {
    fprintf(stderr, "Valid image formats are:\n");
    for (size_t i = 0; i < sizeof(img_formats)/sizeof(img_formats[0]); i++) {
        fprintf(stderr, "  %s\n", img_formats[i].name);
    }
}
int load_image(const char *path, Image *img) {
    img->data = stbi_load(path, &img->width, &img->height, &img->channels, 0);
    if (img->data) {
        img->webp_allocated = 0;
        return 1;
    }

    // If stb_image couldn't detect the format, try libwebp as a fallback
    // (useful when stb_image lacks WebP support in this build)
    fprintf(stderr, "stb failed: %s\n", stbi_failure_reason());

    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    long sz = ftell(f);
    if (sz <= 0) { fclose(f); return 0; }
    rewind(f);

    uint8_t *buf = malloc(sz);
    if (!buf) { fclose(f); return 0; }
    if (fread(buf, 1, sz, f) != (size_t)sz) { free(buf); fclose(f); return 0; }
    fclose(f);

    int w = 0, h = 0;
    if (!WebPGetInfo(buf, (size_t)sz, &w, &h)) {
        free(buf);
        return 0;
    }

    // Decode to RGBA to preserve alpha when present
    uint8_t *pixels = WebPDecodeRGBA(buf, (size_t)sz, &w, &h);
    free(buf);
    if (!pixels) return 0;

    img->width = w;
    img->height = h;
    img->channels = 4;
    img->data = pixels;
    img->webp_allocated = 1; // ensure free_image uses WebPFree
    return 1;
}

void free_image(Image *img) {
    if (!img || !img->data) return;
    if (img->webp_allocated) {
        WebPFree(img->data);
        img->webp_allocated = 0;
    } else {
        stbi_image_free(img->data);
    }
}

CmdArgs parse_cmd(int argc, char* argv[]){
    CmdArgs args = {NULL, NULL};
    
    if (argc != 4){
        fprintf(stderr, "Usage: %s <input_file> -to <output_format>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    args.input_file_name = argv[1];
    if (strcmp(argv[2], "-to") != 0) {
        fprintf(stderr, "Expected '-to' flag\n");
        exit(EXIT_FAILURE);
    }

    if (match_format(argv[3]) == NULL) {
        fprintf(stderr, "Invalid output format: %s\n", argv[3]);
        list_valid_img_formats();
        exit(EXIT_FAILURE);
    }
    args.output_format = argv[3];

    return args;
}

void make_output_filename(const char *input, const char *fmt, char *out, size_t outsz) {
    const char *dot = strrchr(input, '.');
    size_t len = dot ? (size_t)(dot - input) : strlen(input);

    if (len + 1 + strlen(fmt) + 1 > outsz) {
        fprintf(stderr, "Output filename too long\n");
        exit(1);
    }

    memcpy(out, input, len);
    out[len] = '.';
    strcpy(out + len + 1, fmt);
}
int img_convert(const char *input, const char *output) {
    Image img;
    if (!load_image(input, &img)) return 0;

    const char *ext = strrchr(output, '.');
    if (!ext) return 0;
    ext++;

    for (size_t i = 0; i < sizeof(writers)/sizeof(writers[0]); i++) {
        if (strcmp(ext, writers[i].ext) == 0) {
            int ok = writers[i].fn(output, &img);
            free_image(&img);
            return ok;
        }
    }

    fprintf(stderr, "Unsupported format: %s\n", ext);
    free_image(&img);
    return 0;
}

