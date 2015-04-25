
#include "json_encode.h"


json_object* encode_tags(OsmNode* node) {
    json_object * jtags = json_object_new_object();
    int i;
    for (i=0; i<node->tags_count; i++) {
        OsmTag* tag = node->tags[i];
        json_object_object_add(jtags, tag->key, json_object_new_string(tag->value));
    };
    return jtags;
};

json_object* encode_node(OsmNode* node) {
    json_object* jnode = json_object_new_object();
    json_object_object_add(jnode, "id", json_object_new_int(node->id));
    json_object_object_add(jnode, "lat", json_object_new_double(node->lat));
    json_object_object_add(jnode, "lon", json_object_new_double(node->lon));

    if (node->tags_count > 0) {
        json_object* jtags = encode_tags(node);
        json_object_object_add(jnode, "tags", jtags);
    };
    return jnode;
};

const char* encode_json(json_object* jobj) {
    return json_object_to_json_string(jobj);
};
