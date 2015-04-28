
#include "type_defs.h"


void free_tag(OsmTag* tag) {
    // free(tag->key);
    // free(tag->value);
    free(tag);
}


OsmItem* init_item() {
    OsmItem* item = (OsmItem*)malloc(sizeof(OsmItem));
    item->id = 0;
    item->lat = 0;
    item->lon = 0;
    item->tags_count = 0;
    return item;
}


void free_item(OsmItem* item) {
    int i;
    for (i=0; i<item->tags_count; i++) {
        free_tag(item->tags[i]);
    };
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


Cursor* alloc_cursor() {
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));
    cursor->items_count = 0;
    return cursor;
}


void clear_cursor(Cursor* cursor) {
    cursor->position = -1;

    cursor->items_count = 0;
    // TODO: not here
    cursor->items = (OsmItem**)malloc(sizeof(OsmItem*) * DEFAULT_ITEMS_COUNT);
}


void free_cursor_items(Cursor* cursor) {
    if (cursor->items_count > 0) {
        int i;
        for (i=0; i<cursor->items_count; i++) {
            free_item(cursor->items[i]);
        }
    }
    if (cursor->items) {
        free(cursor->items);
    }
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
