
#include "type_defs.h"


void free_tag(Tag* tag) {
    free(tag->key);
    free(tag->value);
    free(tag);
};

Node* init_node() {
    Node* node = (Node*)malloc(sizeof(Node));
    node->tags_count = 0;
    return node;
};

void free_node(Node* node) {
    int i;
    for (i=0;i<node->tags_count;i++) {
        free_tag(node->tags[i]);
    };
};

void node_add_tag(Node* node, Tag* tag) {
    node->tags_count += 1;
    if (node->tags_count == 1) {
        node->tags = malloc(sizeof(Tag));
    } else {
        node->tags = realloc(node->tags, sizeof(Tag) * node->tags_count);
    }
    node->tags[node->tags_count - 1] = tag;
};

Cursor* init_cursor() {
    Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));
    cursor->nodes_count = 0;
    return cursor;
};

void free_cursor(Cursor* cursor){
    int i;
    for (i=0;i<cursor->nodes_count;i++) {
        free_node(cursor->nodes[i]);
    };
    free(cursor);
};

void cursor_add_node(Cursor* cursor, Node* node){
    cursor->nodes_count += 1
    if (cursor->nodes_count % DEFAULT_NODES_COUNT == 0) {
        if (cursor->nodes_count == 1){
            cursor->nodes = (Node**)malloc(sizeof(Node*)*DEFAULT_NODES_COUNT);
        } else {
            cursor->nodes = realloc(
                cursor->nodes,
                sizeof(Node*)*DEFAULT_NODES_COUNT*(cursor->nodes_count/DEFAULT_NODES_COUNT
            );
        }
    }
};