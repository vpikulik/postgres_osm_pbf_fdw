
#ifndef TYPE_DEFS_INCLUDED
#define TYPE_DEFS_INCLUDED

#define file_size_t unsigned long

#include <stdlib.h>
#include <string.h>

#define ALLOCATED_TAGS 10
#define ALLOCATED_MEMBERS 10
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
    char* role;
    int64_t id;
    OsmItemType type;
} OsmMember;


typedef struct {
    OsmItemType type;
    int64_t id;

    double lat;
    double lon;

    size_t tags_count;
    OsmTag** tags;

    size_t node_refs_count;
    int64_t *node_refs;

    size_t members_count;
    OsmMember **members;

    int32_t version;
    double timestamp;
    int64_t changeset;
    int32_t uid;
    char *user;
    short int visible;
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
void item_add_member(OsmItem *item, OsmMember *member);


Cursor* alloc_cursor();
void clear_cursor(Cursor* cursor);
void free_cursor(Cursor* cursor);
void free_cursor_items(Cursor* cursor);
void free_cursor_strings(Cursor* cursor);
void cursor_add_item(Cursor* cursor, OsmItem* item);
void item_copy_node_refs(OsmItem *item, size_t count, int64_t *node_refs);

#endif
