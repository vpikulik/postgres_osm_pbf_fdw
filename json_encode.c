
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
    jitem = json_object_new_object();
    json_object_object_add(jitem, "id", json_object_new_int(item->id));
    json_object_object_add(jitem, "lat", json_object_new_double(item->lat));
    json_object_object_add(jitem, "lon", json_object_new_double(item->lon));

    if (item->tags_count > 0) {
        jtags = encode_tags(item);
        json_object_object_add(jitem, "tags", jtags);
    };
    const char* j_output = json_object_to_json_string(jitem);
    char* output = (char*)malloc(sizeof(char)*(strlen(j_output)+1));
    strcpy(output, j_output);
    json_object_put(jtags);
    json_object_put(jitem);
    return output;
};
