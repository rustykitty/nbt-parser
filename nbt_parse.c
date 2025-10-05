#define _DEFAULT_SOURCE

#include "nbt_parse.h"
#include "nbt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#ifdef __APPLE__ 

// assuming macOS
#  include <machine/endian.h>

#  define be16toh(x) ntohs(x)
#  define be32toh(x) ntohl(x)
#  define be64toh(x) ntohll(x)

#  define htobe16(x) htons(x)
#  define htobe32(x) htonl(x)
#  define htobe64(x) htonll(x)

#elif defined(_WIN32)
// Windows

#include <winsock2.h>

#  define be16toh(x) ntohs(x)
#  define be32toh(x) ntohl(x)
#  define be64toh(x) ntohll(x)

#  define htobe16(x) htons(x)
#  define htobe32(x) htonl(x)
#  define htobe64(x) htonll(x)

#else
// Assuming Linux or UNIX-like that has endian.h
#  include <endian.h>
#endif


#ifndef NDEBUG
#  define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#  define DEBUG_PRINT(...)
#endif

NamedTag* _parse_named_tag(FILE* file) {
    DEBUG_PRINT("In %s, ftell(file) = %04lx\n", __func__, ftell(file));
    NamedTag* ret = (NamedTag*) calloc(1, sizeof(NamedTag));
    Byte type;
    if (!fread(&type, sizeof(Byte), 1, file)) {
        goto error;
    }
    if (type == TAG_End) {
        *ret = (NamedTag){
            .type = TAG_End,
            .name = (String){
                .length = 0,
                .data = NULL,
            },
        };
        return ret;
    }
    {
        String* name = _parse_string(file);
        if (!name) {
            goto error;
        }
        *ret = (NamedTag){
            .type = (enum TAGType) type,
            .name = *name,
        };
        free(name);
    }

    // TODO: highly repetitive and duplicates _parse_list
    switch (type) {
    case TAG_End:
        #ifdef __GNUC__
        __builtin_unreachable();
        #endif
        break;
    case TAG_Byte:
    {
        Byte value;
        if (!fread(&value, sizeof(Byte), 1, file)) {
            goto error;
        }
        ret->byte_value = value;
        break;
    }
    case TAG_Short:
    {
        Short value;
        if (!fread(&value, sizeof(Short), 1, file)) {
            goto error;
        }
        ret->short_value = be16toh(value);
        break;
    }
    case TAG_Int:
    case TAG_Float:
    {
        Int value;
        if (!fread(&value, sizeof(Int), 1, file)) {
            goto error;
        }
        ret->int_value = be32toh(value);
        break;
    }
    case TAG_Long:
    case TAG_Double:
    {
        Long value;
        if (!fread(&value, sizeof(Long), 1, file)) {
            goto error;
        }
        ret->long_value = be64toh(value);
        break;
    }
    case TAG_Byte_Array:
        ret->byte_array_value = _parse_byte_array(file);
        if (!ret->byte_array_value) {
            goto error;
        }
        break;
    case TAG_String:
        ret->string_value = _parse_string(file);
        if (!ret->string_value) {
            goto error;
        }
        break;
    case TAG_List:
        ret->list_value = _parse_list(file);
        if (!ret->list_value) {
            goto error;
        }
        break;
    case TAG_Compound:
        ret->compound_value = _parse_compound(file);
        if (!ret->compound_value) {
            goto error;
        }
        break;
    case TAG_Int_Array:
        ret->int_array_value = _parse_int_array(file);
        if (!ret->int_array_value) {
            goto error;
        }
        break;
    case TAG_Long_Array:
        ret->long_array_value = _parse_long_array(file);
        if (!ret->long_array_value) {
            goto error;
        }
        break;
    default:
        fprintf(stderr, "Unknown tag type %d\n", type);
        goto error;
        break;
    }

    return ret;
    error:
    free(ret);
    return NULL;
}

NamedTag* parse_named_tag(FILE* file) {
    NamedTag* tag = _parse_named_tag(file);
    if (!tag) {
        if (feof(file)) {
            fprintf(stderr, "Unexpected end of file\n");
        } 
        fprintf(stderr, "Failed to parse NBT data\n"
                            "Errno may provide more information:\n"
                            "Errno %d: %s\n", errno, strerror(errno));
    }
    return tag;
}

NamedTag* parse_named_tag_from_buffer(char* buffer, size_t length) {
    FILE* stream = fmemopen((void*)buffer, length, "r");
    if (!stream) {
        return NULL;
    }
    NamedTag* tag = parse_named_tag(stream);
    if (!tag) {
        fprintf(stderr, "An error occurred. ftell(stream) = %ld\n", ftell(stream));
    }
    if (ferror(stream)) {
        fprintf(stderr, "ferror returned true, errno may be set:\n"
                        "Errno %d: %s\n", errno, strerror(errno));
    }
    fclose(stream);
    return tag;
}

Byte_Array* _parse_byte_array(FILE* file) {
    DEBUG_PRINT("In %s, ftell(file) = %04lx\n", __func__, ftell(file));
    Int length;
    if (!fread(&length, sizeof(Int), 1, file)) {
        return NULL;
    }
    length = be32toh(length);
    Byte_Array* ret = (Byte_Array*) calloc(1, sizeof(Byte_Array));
    if (!ret) {
        return NULL;
    }
    ret->length = length;
    ret->data = (Byte*) calloc(length, sizeof(Byte));
    if (!ret->data) {
        free(ret);
        return NULL;
    }
    if (!fread(ret->data, sizeof(Byte), length, file)) {
        free (ret->data);
        free (ret);
        return NULL;
    }
    return ret;
}

String* _parse_string(FILE* file) {
    DEBUG_PRINT("In %s, ftell(file) = %04lx\n", __func__, ftell(file));
    Short length;
    if (!fread(&length, sizeof(Short), 1, file)) {
        return NULL;
    }
    fprintf(stderr, "Length before conversion: %04x\n", length);
    length = be16toh(length);
    char* str;
    if (length == 0) {
        str = "";
    } else {
        str = (char*) calloc(length + 1, sizeof(char));
        if (!str) {
            return NULL;
        }
        if (!fread(str, sizeof(char), length, file)) {
            free(str);
            return NULL;
        }
    }

    String* ret = (String*) calloc(1, sizeof(String));
    if (!ret) {
        free(str);
        return NULL;
    }
    *ret = (String){
        .length = length,
        .data = str
    };
    return ret;
}

List* _parse_list(FILE* file) {
    DEBUG_PRINT("In %s, ftell(file) = %04lx\n", __func__, ftell(file));
    Byte type;
    if (!fread(&type, sizeof(Byte), 1, file)) {
        return NULL;
    }
    Int length;
    if (!fread(&length, sizeof(Int), 1, file)) {
        return NULL;
    }
    length = be32toh(length);
    List* ret = (List*) calloc(1, sizeof(List));
    void* data = calloc(sizeof_type[type], length + 1);
    (void)data;
    for (Int i = 0; i < length; i++) {
        // TODO: highly repetitive and duplicates _parse_named_tag
        // I hope GCC hoists this switch
        switch (type) {
            case TAG_End:
                fprintf(stderr, "%s cannot be the type of a list\n", tag_name[type]);
                goto error;
            case TAG_Byte:
                if (!fread(&((Byte*)data)[i], sizeof(Byte), 1, file)) {
                    goto error;
                }
                break;
            case TAG_Short:
            {
                Short value;
                if (!fread(&value, sizeof(Short), 1, file)) {
                    goto error;
                }
                ((Short*)data)[i] = be16toh(value);
                break;
            }
            case TAG_Int:
            {
                Int value;
                if (!fread(&value, sizeof(Int), 1, file)) {
                    goto error;
                }
                ((Int*)data)[i] = be32toh(value);
                break;
            }
            case TAG_Long:
            {
                Long value;
                if (!fread(&value, sizeof(Long), 1, file)) {
                    goto error;
                }
                ((Long*)data)[i] = be64toh(value);
                break;
            }
            case TAG_Float:
            {
                Float value;
                if (!fread(&value, sizeof(Float), 1, file)) {
                    goto error;
                }
                ((Float*)data)[i] = be32toh(value);
                break;
            }
            case TAG_Double:
            {
                Double value;
                if (!fread(&value, sizeof(Double), 1, file)) {
                    goto error;
                }
                ((Double*)data)[i] = be64toh(value);
                break;
            }

            case TAG_Byte_Array:
            {
                Byte_Array* res = _parse_byte_array(file);
                if (!res) {
                    goto error;
                }
                ((Byte_Array*)data)[i] = *res;
                free(res);
                break;
            }
            case TAG_String:
            {
                String* res = _parse_string(file);
                if (!res) {
                    goto error;
                }
                ((String*)data)[i] = *res;
                free(res);
                break;
            }
            case TAG_List:
            {
                List* res = _parse_list(file);
                if (!res) {
                    goto error;
                }
                ((List*)data)[i] = *res;
                free(res);
                break;
            }
            case TAG_Compound:
            {
                Compound* res = _parse_compound(file);
                if (!res) {
                    goto error;
                }
                ((Compound*)data)[i] = *res;
                free(res);
                break;
            }
            case TAG_Int_Array:
            {
                Int_Array* res = _parse_int_array(file);
                if (!res) {
                    goto error;
                }
                ((Int_Array*)data)[i] = *res;
                free(res);
                break;
            }
            case TAG_Long_Array:
            {
                Long_Array* res = _parse_long_array(file);
                if (!res) {
                    goto error;
                }
                ((Long_Array*)data)[i] = *res;
                free(res);
                break;
            }
            default:
                fprintf(stderr, "Unknown list type %d\n", type);
                goto error;
                break;
        }
    }

    *ret = (List){
        .type = type,
        .length = length,
        .tags = data 
    };

    return ret;

    error:
    free(ret);
    return NULL;
}

Compound* _parse_compound(FILE* file) {
    DEBUG_PRINT("In %s, ftell(file) = %04lx\n", __func__, ftell(file));
    static const size_t INITIAL_CAPACITY = 64;

    // dynamic array init
    size_t array_size = 0;
    size_t array_capacity = INITIAL_CAPACITY;
    NamedTag* array = (NamedTag*) calloc(array_capacity, sizeof(NamedTag));
    if (!array) {
        return NULL;
    }

    while (1) {
        NamedTag* tag = _parse_named_tag(file);
        if (!tag) {
            free(array);
            return NULL;
        }
        if (array_size >= array_capacity) {
            array_capacity *= 2;
            NamedTag* temp = (NamedTag*) realloc(array, array_capacity * sizeof(NamedTag));
            if (!temp) {
                free(array);
                return NULL;
            }
            array = temp;
        }
        array[array_size] = *tag;
        free(tag);
        
        if (array[array_size].type == TAG_End) break;
        array_size++;
    }

    {
        NamedTag* temp = (NamedTag*) realloc(array, (array_size + 1) * sizeof(NamedTag));
        if (!temp) {
            free(array);
            return NULL;
        }
        array = temp;
    }

    Compound* ret = (Compound*) calloc(1, sizeof(Compound));
    if (!ret) {
        free(array);
        return NULL;
    }
    ret->size = array_size;
    ret->tags = array;
    return ret;
}

Int_Array* _parse_int_array(FILE* file) {
    DEBUG_PRINT("In %s, ftell(file) = %04lx\n", __func__, ftell(file));
    Int length;
    if (!fread(&length, sizeof(Int), 1, file)) {
        return NULL;
    }
    length = be32toh(length);
    Int_Array* ret = (Int_Array*) calloc(1, sizeof(Int_Array));
    if (!ret) {
        return NULL;
    }
    ret->length = length;
    ret->data = (Int*) calloc(length, sizeof(Int));
    if (!ret->data) {
        free(ret);
        return NULL;
    }
    if (!fread(ret->data, sizeof(Int), length, file)) {
        free(ret->data);
        free(ret);
        return NULL;
    }
    for (Int i = 0; i < length; i++) {
        ret->data[i] = be32toh(ret->data[i]);
    }
    return ret;
}

Long_Array* _parse_long_array(FILE* file) {
    DEBUG_PRINT("In %s, ftell(file) = %04lx\n", __func__, ftell(file));
    Int length;
    if (!fread(&length, sizeof(Int), 1, file)) {
        return NULL;
    }
    length = be32toh(length);
    Long_Array* ret = (Long_Array*) calloc(1, sizeof(Long_Array));
    if (!ret) {
        return NULL;
    }
    ret->length = length;
    ret->data = (Long*) calloc(length, sizeof(Long));
    if (!ret->data) {
        free(ret);
        return NULL;
    }
    if (!fread(ret->data, sizeof(Long), length, file)) {
        free(ret->data);
        free(ret);
        return NULL;
    }
    for (Int i = 0; i < length; i++) {
        ret->data[i] = be64toh(ret->data[i]);
    }
    return ret;
}
