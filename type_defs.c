
#include "type_defs.h"


void free_tag(OsmTag* tag) {
    // free(tag->key);
    // free(tag->value);
    free(tag);
}

void free_member(OsmMember* member) {
    free(member);
}


OsmItem* init_item() {
    OsmItem* item = (OsmItem*)malloc(sizeof(OsmItem));
    item->type = UNDEFINED;
    item->id = 0;
    item->lat = 0;
    item->lon = 0;
    item->tags_count = 0;
    item->node_refs_count = 0;
    item->members_count = 0;
    return item;
}


void free_item(OsmItem* item) {
    int i;
    if (item->tags_count > 0) {
        for (i=0; i<item->tags_count; i++) {
            free_tag(item->tags[i]);
        }
        free(item->tags);
    }
    if (item->members_count > 0) {
        for (i=0; i<item->members_count; i++) {
            free_member(item->members[i]);
        }
        free(item->members);
    }
    if (item->node_refs_count > 0) {
        free(item->node_refs);
    }
    free(item);
}


void item_add_tag(OsmItem* item, OsmTag* tag) {
    item->tags_count += 1;
    if (item->tags_count == 1) {
        item->tags = (OsmTag**)malloc(sizeof(OsmTag*));
    } else {
        OsmTag** tags = realloc(item->tags, sizeof(OsmTag*) * item->tags_count);
        item->tags = tags;
    };
    item->tags[item->tags_count - 1] = tag;
}


void item_add_member(OsmItem *item, OsmMember *member) {
    item->members_count += 1;
    if (item->members_count == 1) {
        item->members = (OsmMember**)malloc(sizeof(OsmMember*));
    } else {
        item->members = realloc(item->members, sizeof(OsmMember*) * item->members_count);
    };
    item->members[item->members_count - 1] = member;
}


void item_copy_node_refs(OsmItem *item, size_t count, int64_t *node_refs) {
    int i;
    int64_t ref;
    item->node_refs_count = count;
    item->node_refs = (int64_t*)malloc(sizeof(int64_t)*count);
    ref = 0;
    for (i=0; i<count; i++) {
        ref += node_refs[i];
        item->node_refs[i] = ref;
    }
}


Cursor* alloc_cursor() {
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));
    cursor->items_count = 0;
    cursor->items = NULL;
    return cursor;
}


void clear_cursor(Cursor* cursor) {
    free_cursor_items(cursor);
    free_cursor_strings(cursor);
    cursor->items_count = 0;
    cursor->items = (OsmItem**)malloc(sizeof(OsmItem*) * DEFAULT_ITEMS_COUNT);
}


void free_cursor_items(Cursor* cursor) {
    if (cursor->items_count > 0) {
        int i;
        for (i=0; i<cursor->items_count; i++) {
            free_item(cursor->items[i]);
        }
    }
    free(cursor->items);
}


void free_cursor_strings(Cursor* cursor) {
    if (cursor->strings_count > 0) {
        int i;
        for (i=0; i<cursor->strings_count; i++) {
            free(cursor->strings[i]);
        }
    }
    free(cursor->strings);
}


void free_cursor(Cursor* cursor) {
    free_cursor_items(cursor);
    free_cursor_strings(cursor);
    free(cursor);
}


void cursor_add_item(Cursor* cursor, OsmItem* item){
    cursor->items_count += 1;
    cursor->items[cursor->items_count-1] = item;
}
