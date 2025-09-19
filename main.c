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
#include "nbt_parser.h"

static void _traverse(NamedTag* root, const int level);

void traverse(NamedTag* root) {
    return _traverse(root, 0);
}

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

static inline void _indent(int spaces) {
    printf("%*s", spaces, "");
}

static void _traverse(NamedTag* root, const int level) {

    if (!root) return;
    _indent(level * 4);
    printf("%s(\"%s\")", tag_name[root->type], root->name.data ? (char*)root->name.data : "");
    
    switch (root->type) {
        struct {
            Int length;
            size_t element_size;
            Byte type;
            const char* format_specifier;
            void* pointer;
        } array_options;

    case TAG_End:
        break;
    case TAG_Byte:
        printf(": %hhd", root->byte_value);
        break;
    case TAG_Short:
        printf(": %hd", root->short_value);
        break;
    case TAG_Int:
        printf(": %d", root->int_value);
        break;
    case TAG_Long:
        printf(": %ld", root->long_value);
        break;
    case TAG_Float:
        printf(": %f", root->float_value);
        break;
    case TAG_Double:
        printf(": %f", root->double_value);
        break;
    case TAG_Byte_Array:
        array_options.length = root->byte_array_value->length;
        array_options.element_size = sizeof(Byte);
        array_options.type = TAG_Byte;
        array_options.format_specifier = "%hhd";
        array_options.pointer = root->byte_array_value->data;
        goto array;
    case TAG_String:
        printf(": \"%s\"", root->string_value->data);
        break;
    case TAG_List:
        printf(": %d entries of type %s\n", root->list_value->length, tag_name[root->list_value->type]);
        // _indent(level * 4);
        // puts("{");
        // for (int i = 0; i < root->list_value->length; ++i) {
        //     _traverse(&root->list_value->tags[i], level + 1);
        // }
        // _indent(level * 4);
        // putchar('}');
        break;
    case TAG_Compound:
        printf(": %d entries\n", root->compound_value->size);
        _indent(level * 4);
        puts("{");
        for (Int i = 0; i < root->compound_value->size; ++i) {
            _traverse(&root->compound_value->tags[i], level + 1);
        }
        _indent(level * 4);
        printf("}");
        break;
    case TAG_Int_Array:
        array_options.length = root->int_array_value->length;
        array_options.element_size = sizeof(Int);
        array_options.type = TAG_Int;
        array_options.format_specifier = "%d";
        array_options.pointer = root->int_array_value->data;
        goto array;
    case TAG_Long_Array:
        array_options.length = root->long_array_value->length;
        array_options.element_size = sizeof(Long);
        array_options.type = TAG_Long;
        array_options.format_specifier = "%ld";
        array_options.pointer = root->long_array_value->data;
        goto array;
    array:
        printf(": %d elements of type %s\n", array_options.length, tag_name[array_options.type]);
        _indent(level * 4);
        puts("{");
        _indent((level + 1) * 4);
        for (int i = 0; i < array_options.length; ++i) {
            putchar(' ');
            // this should be illegal but who cares
            printf(array_options.format_specifier, *(uint8_t*)(array_options.pointer + array_options.element_size * i));
        }
        putchar('\n');
        _indent(level * 4);
        putchar('}');
        break;
    }
    putchar('\n');
}