#include "nbt.h"
#include "nbt_traverse.h"

#include <stdio.h>

static inline void _indent(int spaces) {
    printf("%*s", spaces, "");
}

void traverse(NamedTag* root, const int level) {

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
            traverse(&root->compound_value->tags[i], level + 1);
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