
#include <stdlib.h>
#include <string.h>

#include "fileformat.pb-c.h"
#include "osmformat.pb-c.h"

// #include "type_defs.h"
#include "json_encode.h"
#include "zdecode.h"


OSMPBF__BlobHeader* read_blob_header(FILE *file, int header_size);
ResizedBuffer* read_blob(FILE* file, OSMPBF__BlobHeader* header);
char** read_osm_string_table(OSMPBF__StringTable *stringtable);
OSMPBF__HeaderBlock* read_osm_header_block(Cursor* cursor, ResizedBuffer* data);
double get_lat(int64_t lat, OSMPBF__PrimitiveBlock* primitive_block);
double get_lon(int64_t lon, OSMPBF__PrimitiveBlock* primitive_block);
void read_osm_dense_nodes(Cursor* cursor, OSMPBF__DenseNodes *dense, char** strings, OSMPBF__PrimitiveBlock* primitive_block);
void read_osm_primitive_group(Cursor* cursor, OSMPBF__PrimitiveGroup *primitive_group, char** strings, OSMPBF__PrimitiveBlock* primitive_block);
void read_osm_primitive_block(Cursor* cursor, ResizedBuffer *data);
void fill_cursor(Cursor* cursor, FILE* file, short osm_header);