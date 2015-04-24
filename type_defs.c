
#include "type_defs.h"


void free_tag(OsmTag* tag) {
    // free(tag->key);
    // free(tag->value);
    free(tag);
};

OsmNode* init_node() {
    OsmNode* node = (OsmNode*)malloc(sizeof(OsmNode));
    node->tags_count = 0;
    return node;
};

void free_node(OsmNode* node) {
    int i;
    for (i=0;i<node->tags_count;i++) {
        free_tag(node->tags[i]);
    };
    free(node);
};

void node_add_tag(OsmNode* node, OsmTag* tag) {
    node->tags_count += 1;
    if (node->tags_count == 1) {
        node->tags = (OsmTag**)malloc(sizeof(OsmTag*));
    } else {
        node->tags = realloc(node->tags, sizeof(OsmTag*) * node->tags_count);
    };
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

void cursor_add_node(Cursor* cursor, OsmNode* node){
    if (cursor->nodes_count % DEFAULT_NODES_COUNT == 0) {
        if (cursor->nodes_count == 0){
            cursor->nodes = (OsmNode**)malloc(sizeof(OsmNode*)*DEFAULT_NODES_COUNT);
        } else {
            int count = (cursor->nodes_count+1)/DEFAULT_NODES_COUNT;
            cursor->nodes = realloc(
                cursor->nodes,
                sizeof(OsmNode*) * DEFAULT_NODES_COUNT * count
            );
        };
    };
    cursor->nodes_count += 1;
    cursor->nodes[cursor->nodes_count-1] = node;
};