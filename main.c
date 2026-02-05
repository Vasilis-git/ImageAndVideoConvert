#include "include/tools.h"

int main(int argc, char *argv[]) {
    CmdArgs args = {NULL, NULL};
    char outname[1024];


    args = parse_cmd(argc, argv);
    make_output_filename(args.input_file_name, args.output_format, outname, sizeof(outname));

    if (!img_convert(args.input_file_name, outname)) {
        fprintf(stderr, "Conversion failed\n");
        return 1;
    }

    printf("Created %s\n", outname);
    return 0;
}