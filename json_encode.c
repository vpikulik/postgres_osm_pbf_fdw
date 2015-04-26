
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

json_object* encode_item(OsmItem* item) {
    json_object* jitem = json_object_new_object();
    json_object_object_add(jitem, "id", json_object_new_int(item->id));
    json_object_object_add(jitem, "lat", json_object_new_double(item->lat));
    json_object_object_add(jitem, "lon", json_object_new_double(item->lon));

    if (item->tags_count > 0) {
        json_object* jtags = encode_tags(item);
        json_object_object_add(jitem, "tags", jtags);
    };
    return jitem;
};

const char* encode_json(json_object* jobj) {
    return json_object_to_json_string(jobj);
};
