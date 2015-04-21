
#include <stdlib.h>

#define DEFAULT_NODES_COUNT 8000

typedef struct {
    char* key;
    char* value;
} Tag;

typedef struct {
    int64_t id;
    int64_t lat;
    int64_t lon;
    int32_t tags_count;
    Tag** tags;
} Node;


typedef struct {
    int nodes_count;
    Node** nodes;
} Cursor;

void node_add_tag(Node* node, Tag* tag);
Cursor* init_cursor();
