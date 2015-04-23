
#include <stdlib.h>

#define DEFAULT_NODES_COUNT 8000


typedef struct {
    char* key;
    char* value;
} Tag;


typedef struct {
    int64_t id;
    double lat;
    double lon;
    int32_t tags_count;
    Tag** tags;
} Node;


typedef struct {
    int nodes_count;
    Node** nodes;
} Cursor;


Node* init_node();
void free_node(Node* node);
void node_add_tag(Node* node, Tag* tag);


Cursor* init_cursor();
void free_cursor(Cursor* cursor);
void cursor_add_node(Cursor* cursor, Node* node);
