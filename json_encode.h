
#include <jansson.h>

#include "type_defs.h"


json_t* encode_tag(Tag* tag); 
json_t* encode_node(Node* node);

char* encode_json(json_t* obj);
