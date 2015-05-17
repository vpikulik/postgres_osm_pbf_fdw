
#include "jsonb_encode.h"


JsonbValue* make_jsonb_string_value(char* string) {
    JsonbValue *jvalue = (JsonbValue*)palloc(sizeof(JsonbValue));
    memset(jvalue, 0, sizeof(JsonbValue));
    jvalue->type = jbvString;
    jvalue->val.string.len = strlen(string);
    jvalue->val.string.val = string;
    return jvalue;
}


JsonbValue* make_jsonb_numeric_value(double value) {
    JsonbValue *jvalue = (JsonbValue*)palloc(sizeof(JsonbValue));
    memset(jvalue, 0, sizeof(JsonbValue));
    Datum d_num_value = Float8GetDatum(value);
    Numeric num_value = DatumGetNumeric(DirectFunctionCall1(float8_numeric, d_num_value));
    jvalue->type = jbvNumeric;
    jvalue->val.numeric = num_value;
    return jvalue;
}


JsonbValue* make_jsonb_array(int elems_count, JsonbValue **elems) {
    JsonbValue *jarray = (JsonbValue*)palloc(sizeof(JsonbValue));
    memset(jarray, 0, sizeof(JsonbValue));
    jarray->type = jbvArray;
    int i;
    jarray->val.array.nElems = elems_count;
    jarray->val.array.elems = (JsonbValue*)palloc(sizeof(JsonbValue) * elems_count);
    for (i=0; i<elems_count; i++) {
        jarray->val.array.elems[i] = *(elems[i]);
    }
    return jarray;
}


JsonbValue* make_jsonb_object(int elems_count, JsonbValue **keys, JsonbValue **values) {
    JsonbValue *jobject = (JsonbValue*)palloc(sizeof(JsonbValue));
    memset(jobject, 0, sizeof(JsonbValue));
    jobject->type = jbvObject;
    int i;
    jobject->val.object.nPairs = elems_count;
    jobject->val.object.pairs = (JsonbPair*)palloc(sizeof(JsonbPair) * elems_count);
    for (i=0; i<elems_count; i++) {
        JsonbPair pair;
        pair.key = *(keys[i]);
        pair.value = *(values[i]);
        pair.order = i;
        jobject->val.object.pairs[i] = pair;
    }

    return jobject;
}

Jsonb* jsonb_encode_tags(OsmItem* item) {
    int i;
    JsonbValue **jkeys = (JsonbValue**)palloc(sizeof(JsonbValue*) * item->tags_count);
    JsonbValue **jvalues = (JsonbValue**)palloc(sizeof(JsonbValue*) * item->tags_count);
    for (i=0; i<item->tags_count; i++) {
        OsmTag *tag = item->tags[i];
        jkeys[i] = make_jsonb_string_value(tag->key);
        jvalues[i] = make_jsonb_string_value(tag->value);
    }
    // free data here
    JsonbValue *jtags = make_jsonb_object(item->tags_count, jkeys, jvalues);
    return JsonbValueToJsonb(jtags);
}


static JsonbValue* jsonb_encode_member(OsmMember* member) {
    JsonbValue **jkeys = (JsonbValue**)palloc(sizeof(JsonbValue*) * 3);
    JsonbValue **jvalues = (JsonbValue**)palloc(sizeof(JsonbValue*) * 3);

    char* member_type_name;
    if (member->type == NODE) member_type_name = "NODE";
    else if (member->type == WAY) member_type_name = "WAY";
    else if (member->type == RELATION) member_type_name = "RELATION";

    jkeys[0] = make_jsonb_string_value("role");
    jvalues[0] = make_jsonb_string_value(member->role);
    jkeys[1] = make_jsonb_string_value("type");
    jvalues[1] = make_jsonb_string_value(member_type_name);
    jkeys[2] = make_jsonb_string_value("id");
    jvalues[2] = make_jsonb_numeric_value(member->id);
    // free data here
    return make_jsonb_object(3, jkeys, jvalues);
}


Jsonb* jsonb_encode_members(OsmItem* item) {
    int i;
    JsonbValue **jvalues = (JsonbValue**)palloc(sizeof(JsonbValue*) * item->members_count);
    for (i=0; i<item->members_count; i++) {
        OsmMember* member = item->members[i];
        jvalues[i] = jsonb_encode_member(member);
    }
    // free data here
    JsonbValue *jmembers = make_jsonb_array(item->members_count, jvalues);
    return JsonbValueToJsonb(jmembers);
}
