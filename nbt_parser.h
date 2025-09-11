#include <stdio.h>
#include "nbt.h"

NamedTag* parse_named_tag(FILE*);
NamedTag* parse_named_tag_from_buffer(char*, size_t);

Byte_Array* _parse_byte_array(FILE*);
String* _parse_string(FILE*);
List* _parse_list(FILE*);
Compound* _parse_compound(FILE*);
Int_Array* _parse_int_array(FILE*);
Long_Array* _parse_long_array(FILE*);