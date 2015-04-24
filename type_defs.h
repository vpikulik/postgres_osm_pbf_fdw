
#include <stdlib.h>

#define DEFAULT_NODES_COUNT 8000


typedef struct {
    char* key;
    char* value;
} OsmTag;


typedef struct {
    int64_t id;
    double lat;
    double lon;
    int32_t tags_count;
    OsmTag** tags;
} OsmNode;


typedef struct {
    int nodes_count;
    OsmNode** nodes;
} Cursor;


OsmNode* init_node();
void free_node(OsmNode* node);
void node_add_tag(OsmNode* node, OsmTag* tag);


Cursor* init_cursor();
void free_cursor(Cursor* cursor);
void cursor_add_node(Cursor* cursor, OsmNode* node);
