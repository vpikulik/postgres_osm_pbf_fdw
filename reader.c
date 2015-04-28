#include <stdio.h>

#include "type_defs.h"
#include "osm_reader.h"
#include "json_encode.h"


void print_progress(FILE *file, int file_size) {
    int current_pos = ftell(file);
    printf("Progress: %d from %d\n", current_pos, file_size);
};


int main (int argc, const char * argv[]) {
    FILE *fl = fopen("/home/promo/Downloads/belarus-latest.osm.pbf", "r");
    FILE *out = fopen("/tmp/out.json", "w");

    fseek(fl, 0, SEEK_END);
    int file_size = ftell(fl);
    fseek(fl, 0, SEEK_SET);

    int index = 0;
    int current_pos;
    Cursor* cursor = alloc_cursor();
    clear_cursor(cursor);
    read_osm_header(cursor, fl);

    OsmItem* item;
    do {
        item = read_osm_item(cursor, fl);
        if (item) {
            const char* json_item_txt = encode_json(encode_item(item));
            fputs(json_item_txt, out);
            fputs("\n", out);
        }
        index += 1;
    } while (item != NULL && index < 24000);

    free_cursor(cursor);

    fclose(fl);
    fclose(out);
}
