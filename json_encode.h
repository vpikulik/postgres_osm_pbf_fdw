
#include <json.h>

#include "type_defs.h"


json_object* encode_tags(OsmNode* node);
json_object* encode_node(OsmNode* node);

const char* encode_json(json_object* jobj);
