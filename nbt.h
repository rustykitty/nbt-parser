#ifndef NBT_H
#define NBT_H

#include <stdint.h>
#include <stddef.h>

enum TAGType {
    TAG_End = 0,
    TAG_Byte = 1,
    TAG_Short = 2,
    TAG_Int = 3,
    TAG_Long = 4,
    TAG_Float = 5,
    TAG_Double = 6,
    TAG_Byte_Array = 7,
    TAG_String = 8,
    TAG_List = 9,
    TAG_Compound = 10,
    TAG_Int_Array = 11,
    TAG_Long_Array = 12
};

typedef int8_t Byte;
typedef int16_t Short;
typedef int32_t Int;
typedef int64_t Long;
typedef float Float;
typedef double Double;

typedef struct NamedTag NamedTag;
typedef struct Byte_Array Byte_Array;
typedef struct String String;
typedef struct List List;
typedef struct Compound Compound;
typedef struct Int_Array Int_Array;
typedef struct Long_Array Long_Array;
typedef struct GenericArray GenericArray;

extern const char* tag_name[];
extern const size_t sizeof_type[];

#define TAG_HEAD \
    enum TAGType type; 

struct Byte_Array {
    Int length;
    Byte* data;
};

struct Int_Array {
    Int length;
    Int* data;
};

struct Long_Array {
    Int length;
    Long* data;
};

struct String {
    Short length;
    char* data;
};

struct List {
    enum TAGType type;
    Int length;
    void* tags;
};

// TODO: make this into a hash table
struct Compound {
    Int size;
    NamedTag* tags;
};

struct NamedTag {
    enum TAGType type;
    String name;
    union
    {
        Byte byte_value;
        Short short_value;
        Int int_value;
        Long long_value;
        Float float_value;
        Double double_value;
        struct Byte_Array* byte_array_value;
        struct String* string_value;
        struct List* list_value;
        struct Compound* compound_value;
        struct Int_Array* int_array_value;
        struct Long_Array* long_array_value;
    };
};

struct GenericArray {
    Int length;
    void* data;
};

/* free functions */
void NamedTag_free(NamedTag*);
void Byte_Array_free(Byte_Array*);
void String_free(String*);
void List_free(List*);
void Compound_free(Compound*);
void IntArray_free(Int_Array*);
void LongArray_free(Long_Array*);

/* destructor functions */
void NamedTag_destroy(NamedTag*);
void Byte_Array_destroy(Byte_Array*);
void String_destroy(String*);
void List_destroy(List*);
void Compound_destroy(Compound*);
void IntArray_destroy(Int_Array*);
void LongArray_destroy(Long_Array*);
void _GenericArray_destroy(GenericArray*);

/* traversal/access functions */
NamedTag* Compound_find(Compound*, const char*);

#endif // NBT_H