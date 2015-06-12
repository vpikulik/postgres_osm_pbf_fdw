
#ifndef OSM_READER_INCLUDED
#define OSM_READER_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fileformat.pb-c.h"
#include "osmformat.pb-c.h"

#include "type_defs.h"
#include "zdecode.h"


OSMPBF__BlobHeader* read_blob_header(FILE *file, int header_size);
ResizedBuffer* read_blob(FILE* file, OSMPBF__BlobHeader* header);
void read_osm_string_table(Cursor* cursor, OSMPBF__StringTable *stringtable);
OSMPBF__HeaderBlock* read_osm_header_block(Cursor* cursor, ResizedBuffer* data);
void read_osm_dense_nodes(Cursor* cursor, OSMPBF__DenseNodes *dense, char** strings, OSMPBF__PrimitiveBlock* primitive_block);
void read_osm_way(Cursor* cursor, OSMPBF__Way *way, char** strings, OSMPBF__PrimitiveBlock* primitive_block);
void read_osm_relation(Cursor* cursor, OSMPBF__Relation *relation, char** strings, OSMPBF__PrimitiveBlock* primitive_block);
void read_osm_primitive_group(Cursor* cursor, OSMPBF__PrimitiveGroup *primitive_group, char** strings, OSMPBF__PrimitiveBlock* primitive_block);
void read_osm_primitive_block(Cursor* cursor, ResizedBuffer *data);

void read_osm_header(Cursor* cursor, FILE* file);
OsmItem* read_osm_item(Cursor* cursor, FILE* file, int file_size);
int get_osm_item_count(FILE* file, int file_size);

#endif
