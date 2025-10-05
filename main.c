#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>
#include <assert.h>

// hackish way to not get a compilation error because of Byte typedef
#define Byte Zlib_Byte
#include "zpipe.h"
#undef Byte

#pragma GCC diagnostic pop

#include "nbt.h"
#include "nbt_parse.h"
#include "nbt_traverse.h"

#define traverse(root) traverse(root, 0)

int main(int argc, char* argv[]) {
    static const char* default_file = "./nbt/bigtest.nbt";

    const char* filename = argc > 1 ? argv[1] : default_file;

    FILE* input = fopen(filename, "rb");
    if (!input) {
        perror(filename);
        return 1;
    }

    bool is_gzipped;
    bool is_deflated;
    {
        // cast to char because C++11 narrowing conversions
        char gzip_magic[2] = { 0x1f, (char)0x8b };
        char deflate_magic = 0x78;
        char buf[2];
        if (!fread(buf, 2, 1, input)) {
            if (ferror(input)) {
                perror("Error reading input file");
            } else if (feof(input)) {
                fprintf(stderr, "Unexpected end of file\n");
            }
            fclose(input);
            return 1;
        }
        is_gzipped = memcmp(buf, gzip_magic, 2) == 0;
        is_deflated = buf[0] == deflate_magic;
        fseek(input, 0, SEEK_SET);
    }

    assert(!(is_gzipped && is_deflated));

    char* buffer = NULL;
    size_t buffer_length = 0;

    NamedTag* tag;

    if (is_gzipped || is_deflated) {
        FILE* stream = open_memstream(&buffer, &buffer_length);

        int ret = inf(input, stream, is_gzipped);
        if (ret != Z_OK) {
            zerr(ret);
            return 1;
        }

        fclose(stream);
        tag = parse_named_tag_from_buffer(buffer, buffer_length);
    } else {
        // tag = parse_named_tag(stdin);
        fseek(input, 0, SEEK_END);
        buffer_length = ftell(input);
        fseek(input, 0, SEEK_SET);
        buffer = (char*) malloc(buffer_length);
        if (!buffer) {
            perror("Memory allocation failed");
            return 1;
        }
        if (!fread(buffer, 1, buffer_length, input)) {
            perror("Error reading file");
            return 1;
        }
        tag = parse_named_tag_from_buffer(buffer, buffer_length);
    }

    fclose(input);


    if (!tag) {
        free(buffer);
        return 1;
    }

    traverse(tag);

    free(buffer);
    NamedTag_free(tag);

    return 0;
}

#undef traverse