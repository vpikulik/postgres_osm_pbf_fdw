
#include "json_encode.h"


json_object* encode_tags(OsmItem* item) {
    json_object * jtags = json_object_new_object();
    int i;
    for (i=0; i<item->tags_count; i++) {
        OsmTag* tag = item->tags[i];
        json_object_object_add(jtags, tag->key, json_object_new_string(tag->value));
    };
    return jtags;
};


char* encode_item(OsmItem* item) {
    json_object *jitem, *jtags;
    char* type_name;
    int i;

    jitem = json_object_new_object();

    if (item->type == NODE) type_name = "NODE";
    else if (item->type == WAY) type_name = "WAY";
    else if (item->type == RELATION) type_name = "RELATION";

    json_object_object_add(jitem, "type", json_object_new_string(type_name));
    json_object_object_add(jitem, "id", json_object_new_int64(item->id));

    if (item->lat > 0 || item->lon > 0) {
        json_object_object_add(jitem, "lat", json_object_new_double(item->lat));
        json_object_object_add(jitem, "lon", json_object_new_double(item->lon));
    }

    if (item->tags_count > 0) {
        jtags = encode_tags(item);
        json_object_object_add(jitem, "tags", jtags);
    }

    if (item->node_refs_count > 0) {
        json_object *jrefs = json_object_new_array();
        for (i=0; i<item->node_refs_count; i++) {
            json_object *jref_str = json_object_new_int64(item->node_refs[i]);
            json_object_array_add(jrefs, jref_str);
        }
        json_object_object_add(jitem, "refs", jrefs);
    }

    const char* j_output = json_object_to_json_string(jitem);
    char* output = (char*)malloc(sizeof(char)*(strlen(j_output)+1));
    strcpy(output, j_output);
    json_object_put(jitem);
    return output;
};
