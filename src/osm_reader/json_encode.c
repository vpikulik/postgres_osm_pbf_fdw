
#include "json_encode.h"


json_object* prepare_tags(OsmItem* item) {
    json_object *jtags = json_object_new_object();
    int i;
    for (i=0; i<item->tags_count; i++) {
        OsmTag *tag = item->tags[i];
        json_object_object_add(jtags, tag->key, json_object_new_string(tag->value));
    };
    return jtags;
};


json_object* prepare_members(OsmItem* item) {
    json_object *jmembers = json_object_new_array();
    char* member_type_name;
    int i;
    for (i=0; i<item->members_count; i++) {
        OsmMember* member = item->members[i];

        if (member->type == NODE) member_type_name = "NODE";
        else if (member->type == WAY) member_type_name = "WAY";
        else if (member->type == RELATION) member_type_name = "RELATION";

        json_object *jmember = json_object_new_object();
        json_object_object_add(jmember, "role", json_object_new_string(member->role));
        json_object_object_add(jmember, "type", json_object_new_string(member_type_name));
        json_object_object_add(jmember, "id", json_object_new_int64(member->id));
        json_object_array_add(jmembers, jmember);
    }
    return jmembers;
}


char* encode_tags(OsmItem* item) {
    json_object *jtags;
    jtags = prepare_tags(item);

    const char* j_output = json_object_to_json_string(jtags);
    char* output = (char*)malloc(sizeof(char)*(strlen(j_output)+1));
    strcpy(output, j_output);
    json_object_put(jtags);
    return output;
}


char* encode_members(OsmItem* item) {
    json_object *jmembers;
    jmembers = prepare_members(item);

    const char* j_output = json_object_to_json_string(jmembers);
    char* output = (char*)malloc(sizeof(char)*(strlen(j_output)+1));
    strcpy(output, j_output);
    json_object_put(jmembers);
    return output;
}


char* encode_item(OsmItem* item) {
    json_object *jitem, *jtags, *jmembers;
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
        jtags = prepare_tags(item);
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

    if (item->members_count > 0) {
        jmembers = prepare_members(item);
        json_object_object_add(jitem, "members", jmembers);
    }

    time_t changed = (time_t) item->timestamp;
    char changed_date[80];
    struct tm ts;
    ts = *gmtime(&changed);
    strftime(changed_date, sizeof(changed_date), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

    json_object_object_add(jitem, "version", json_object_new_int(item->version));
    json_object_object_add(jitem, "timestamp", json_object_new_string(changed_date));
    json_object_object_add(jitem, "changeset", json_object_new_int64(item->changeset));
    json_object_object_add(jitem, "uid", json_object_new_int(item->uid));
    // char* user = (char*)malloc(sizeof(char)*(strlen(item->user)+1));
    // memcpy(user, item->user, strlen(item->user));
    if (item->user != "") {
        json_object_object_add(jitem, "user", json_object_new_string(item->user));
    }
    json_object_object_add(jitem, "visible", json_object_new_int(item->visible));

    const char* j_output = json_object_to_json_string(jitem);
    char* output = (char*)malloc(sizeof(char)*(strlen(j_output)+1));
    strcpy(output, j_output);
    json_object_put(jitem);
    return output;
};
