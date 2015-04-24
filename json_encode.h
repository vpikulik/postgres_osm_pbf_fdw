
#include <jansson.h>

#include "type_defs.h"


json_t* encode_tag(OsmTag* tag); 
json_t* encode_node(OsmNode* node);

char* encode_json(json_t* obj);
