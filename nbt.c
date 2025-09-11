#include "nbt.h"

#include <stdlib.h>
#include <string.h>

const char* tag_name[] = {
    [TAG_End] = "TAG_End",
    [TAG_Byte] = "TAG_Byte",
    [TAG_Short] = "TAG_Short",
    [TAG_Int] = "TAG_Int",
    [TAG_Long] = "TAG_Long",
    [TAG_Float] = "TAG_Float",
    [TAG_Double] = "TAG_Double",
    [TAG_Byte_Array] = "TAG_Byte_Array",
    [TAG_String] = "TAG_String",
    [TAG_List] = "TAG_List",
    [TAG_Compound] = "TAG_Compound",
    [TAG_Int_Array] = "TAG_Int_Array",
    [TAG_Long_Array] = "TAG_Long_Array"
};

const size_t sizeof_type[] = {
    [TAG_End] = 0,
    [TAG_Byte] = sizeof(Byte),
    [TAG_Short] = sizeof(Short),
    [TAG_Int] = sizeof(Int),
    [TAG_Long] = sizeof(Long),
    [TAG_Float] = sizeof(Float),
    [TAG_Double] = sizeof(Double),
    [TAG_Byte_Array] = sizeof(Byte_Array),
    [TAG_String] = sizeof(String),
    [TAG_List] = sizeof(List),
    [TAG_Compound] = sizeof(Compound),
    [TAG_Int_Array] = sizeof(Int_Array),
    [TAG_Long_Array] = sizeof(Long_Array)
};

/* free functions */
void NamedTag_free(NamedTag* tag) {
    NamedTag_destroy(tag);
    free(tag);
}

void Byte_Array_free(Byte_Array* arr) {
    Byte_Array_destroy(arr);
    free(arr);
}

void String_free(String* str) {
    String_destroy(str);
    free(str);
}

void List_free(List* list) {
    List_destroy(list);
    free(list);
}

void Compound_free(Compound* obj) {
    Compound_destroy(obj);
    free(obj);
}

void IntArray_free(Int_Array* arr) {
    IntArray_destroy(arr);
    free(arr);
}

void LongArray_free(Long_Array* arr) {
    LongArray_destroy(arr);
    free(arr);
}


/* destructor functions */

void NamedTag_destroy(NamedTag* tag) {
    // free any pointers first
    switch (tag->type) {
    case TAG_End:
    case TAG_Byte:
    case TAG_Short:
    case TAG_Int:
    case TAG_Long:
    case TAG_Double:
    case TAG_Float:
        break; // no additional free needed for primitives
    case TAG_Byte_Array:
        Byte_Array_free(tag->byte_array_value);
        break;
    case TAG_String:
        String_free(tag->string_value);
        break;
    case TAG_List:
        List_free(tag->list_value);
        break;
    case TAG_Compound:
        Compound_free(tag->compound_value);
        break;
    case TAG_Int_Array:
        IntArray_free(tag->int_array_value);
        break;
    case TAG_Long_Array:
        LongArray_free(tag->long_array_value);
        break;
    }
    String_destroy(&tag->name);
}

void Byte_Array_destroy(Byte_Array* arr) {
    return _GenericArray_destroy((GenericArray*) arr);
}

void String_destroy(String* str) {
    free(str->data);
}

void List_destroy(List* list) {
    switch (list->type) {
    case TAG_End:
    case TAG_Byte:
    case TAG_Short:
    case TAG_Int:
    case TAG_Long:
    case TAG_Double:
    case TAG_Float:
        break; // no additional free needed for primitives
    case TAG_Byte_Array: {
        Byte_Array* data = (Byte_Array*)(list->tags);
        for (Int i = 0; i < list->length; ++i) {
            Byte_Array_destroy(&data[i]);
        }
        break;
    }
    case TAG_String: {
        String* data = (String*)(list->tags);
        for (Int i = 0; i < list->length; ++i) {
            String_destroy(&data[i]);
        }
        break;
    }
    case TAG_List: {
        String* data = (String*)(list->tags);
        for (Int i = 0; i < list->length; ++i) {
            String_destroy(&data[i]);
        }
        break;
    }
    case TAG_Compound: {
        Compound* data = (Compound*)(list->tags);
        for (Int i = 0; i < list->length; ++i) {
            Compound_destroy(&data[i]);
        }
        break;
    }
    case TAG_Int_Array: {
        Int_Array* data = (Int_Array*)(list->tags);
        for (Int i = 0; i < list->length; ++i) {
            IntArray_destroy(&data[i]);
        }
        break;
    }
    case TAG_Long_Array: {
        Long_Array* data = (Long_Array*)(list->tags);
        for (Int i = 0; i < list->length; ++i) {
            LongArray_destroy(&data[i]);
        }
        break;
    }
    default:;
    }
    free(list->tags);
}

void Compound_destroy(Compound* obj) {
    for (Int i = 0; i < obj->size; ++i) {
        NamedTag_destroy(&obj->tags[i]);
    }
    free(obj->tags);
}

void IntArray_destroy(Int_Array* arr) {
    return _GenericArray_destroy((GenericArray*) arr);
}

void LongArray_destroy(Long_Array* arr) {
    return _GenericArray_destroy((GenericArray*) arr);
}

void _GenericArray_destroy(GenericArray* arr) {
    free(arr->data);
}

NamedTag* Compound_find(Compound* obj, const char* key) {
    for (Int i = 0; i < obj->size; ++i) {
        NamedTag* tag = &obj->tags[i];
        if (strcmp(tag->name.data, key) == 0) {
            return tag;
        }
    }
    return NULL;
}
