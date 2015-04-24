#include <stdio.h>

#include "osm_reader.h"


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
    for (index=0; index<3; index++){
        printf("Index: %d\n", index);
        print_progress(fl, file_size);
        current_pos = ftell(fl);

        if (current_pos >= file_size) {
            printf("End of file\n");
            break;
        }

        Cursor* cursor = init_cursor();
        fill_cursor(cursor, fl, index == 0);

        int i;
        for(i=0; i<cursor->nodes_count; i++) {
            OsmNode* node = cursor->nodes[i];
            json_t* jnode = encode_node(node);
            char* out_str = encode_json(jnode);
            fputs(out_str, out);
            fputs("\n", out);
        }

        free_cursor(cursor);
    }

    fclose(fl);
    fclose(out);
}
