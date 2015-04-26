
#include <json.h>

#include "type_defs.h"


json_object* encode_tags(OsmItem* item);
json_object* encode_item(OsmItem* item);

const char* encode_json(json_object* jobj);
