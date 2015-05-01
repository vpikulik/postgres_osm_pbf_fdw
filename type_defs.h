
#ifndef TYPE_DEFS_INCLUDED
#define TYPE_DEFS_INCLUDED

#include <stdlib.h>

#define DEFAULT_ITEMS_COUNT 8000


typedef struct {
    char* key;
    char* value;
} OsmTag;


typedef enum {
    UNDEFINED = 0,
    NODE = 1,
    WAY = 2,
    RELATION = 3
} OsmItemType;


typedef struct {
    OsmItemType type;
    int64_t id;
    double lat;
    double lon;
    int32_t tags_count;
    OsmTag** tags;
} OsmItem;


typedef struct {
    int position;
    int items_count;
    OsmItem** items;
    int strings_count;
    char** strings;
} Cursor;


OsmItem* init_item();
void free_item(OsmItem* item);
void item_add_tag(OsmItem* node, OsmTag* tag);


Cursor* alloc_cursor();
void clear_cursor(Cursor* cursor);
void free_cursor(Cursor* cursor);
void free_cursor_items(Cursor* cursor);
void free_cursor_strings(Cursor* cursor);
void cursor_add_item(Cursor* cursor, OsmItem* item);

#endif
