
#include "json_encode.h"


json_t* encode_tag(OsmTag* tag) {
    json_t* jtag = json_object();
    json_object_set(jtag, tag->key, json_string(tag->value));
    return jtag;
};

json_t* encode_node(OsmNode* node) {
    json_t* jnode = json_object();
    json_object_set(jnode, "id", json_integer(node->id));
    json_object_set(jnode, "lat", json_real(node->lat));
    json_object_set(jnode, "lon", json_real(node->lon));

    if (node->tags_count > 0) {
        json_t* jtags = json_object();
        int i;
        for (i=0; i<node->tags_count; i++) {
            OsmTag* tag = node->tags[i];
            json_object_set(jtags, tag->key, json_string(tag->value));
        };
        json_object_set(jnode, "tags", jtags);
    };
    return jnode;
};

char* encode_json(json_t* obj) {
    return json_dumps(obj, JSON_INDENT(4));
};
