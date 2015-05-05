
#include "postgres.h"
#include "catalog/pg_type.h"
#include "foreign/fdwapi.h"
#include "optimizer/cost.h"
#include "optimizer/pathnode.h" // create_foreignscan_path
#include "optimizer/planmain.h" // make_foreignscan
#include "optimizer/restrictinfo.h" // extract_actual_clauses
#include "utils/array.h"
#include "utils/json.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;


#include "osm_reader.h"
#include "json_encode.h"

#define ROWS_COUNT 8000*1000


typedef struct FdwExecutionState
{
    FILE* file;
    Cursor* cursor;
    int file_size;
} FdwExecutionState;


void
GetForeignRelSize (PlannerInfo *root,
                   RelOptInfo *baserel,
                   Oid foreigntableid) {
    baserel->rows = ROWS_COUNT;
};

void
GetForeignPaths (PlannerInfo *root,
                 RelOptInfo *baserel,
                 Oid foreigntableid) {

    Cost startup_cost = baserel->baserestrictcost.startup;
    Cost run_cost = 0;
    Cost cpu_per_tuple = cpu_tuple_cost * 10 + baserel->baserestrictcost.per_tuple;

    // run_cost += seq_page_cost * pages;
    run_cost += cpu_per_tuple * ROWS_COUNT;

    Cost total_cost = startup_cost + run_cost;

    add_path(
        baserel,
        (Path *) create_foreignscan_path(
            root,
            baserel,
            baserel->rows,
            startup_cost,
            total_cost,
            NIL,
            NULL,
            baserel->fdw_private
        )
    );
};

ForeignScan *
GetForeignPlan (PlannerInfo *root,
                RelOptInfo *baserel,
                Oid foreigntableid,
                ForeignPath *best_path,
                List *tlist,
                List *scan_clauses) {

    scan_clauses = extract_actual_clauses(scan_clauses, false);

    return make_foreignscan(
        tlist,
        scan_clauses,
        baserel->relid,
        NIL,
        best_path->fdw_private);
};

void
BeginForeignScan (ForeignScanState *node, int eflags) {
    FdwExecutionState *state = (FdwExecutionState*) palloc(sizeof(FdwExecutionState));

    FILE *file = fopen("/home/promo/Downloads/monaco-latest.osm.pbf", "r");
    fseek(file, 0, SEEK_END);
    state->file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    state->file = file;

    state->cursor = alloc_cursor();
    clear_cursor(state->cursor);
    state->cursor->position = -1;
    read_osm_header(state->cursor, state->file);

    node->fdw_state = (void*)state;
};

TupleTableSlot *
IterateForeignScan (ForeignScanState *node){
    OsmItem* item;
    FdwExecutionState *state = node->fdw_state;

    item = read_osm_item(state->cursor, state->file, state->file_size);
    if (item == NULL) {
        return NULL;
    }

    TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
    ExecClearTuple(slot);

    // columns: id, type, lat, lon, tags

    int64 p_id = (int64)item->id;
    slot->tts_values[0] = Int64GetDatum(p_id);
    slot->tts_isnull[0] = false;

    text *type_name;
    if (item->type == NODE) type_name = cstring_to_text("NODE");
    else if (item->type == WAY) type_name = cstring_to_text("WAY");
    else if (item->type == RELATION) type_name = cstring_to_text("RELATION");
    slot->tts_values[1] = PointerGetDatum(type_name);
    slot->tts_isnull[1] = false;

    float8 p_lat = (float8)item->lat;
    if (p_lat) {
        slot->tts_values[2] = Float8GetDatum(p_lat);
        slot->tts_isnull[2] = false;
    } else {
        slot->tts_values[2] = PointerGetDatum(NULL);
        slot->tts_isnull[2] = true;
    }

    float8 p_lon = (float8)item->lon;
    if (p_lon) {
        slot->tts_values[3] = Float8GetDatum(p_lon);
        slot->tts_isnull[3] = false;
    } else {
        slot->tts_values[3] = PointerGetDatum(NULL);
        slot->tts_isnull[3] = true;
    }

    if (item->tags_count > 0) {
        text *tags_json = cstring_to_text(encode_tags(item));
        slot->tts_values[4] = PointerGetDatum(tags_json);
        slot->tts_isnull[4] = false;
    } else {
        slot->tts_values[4] = PointerGetDatum(NULL);
        slot->tts_isnull[4] = true;
    }

    if (item->node_refs_count > 0) {
        Datum *node_refs_array = (Datum*)palloc(sizeof(Datum) * item->node_refs_count);
        int i;
        for (i=0; i<item->node_refs_count; i++) {
            node_refs_array[i] = Int64GetDatum(item->node_refs[i]);
        }
        slot->tts_values[5] = PointerGetDatum(construct_array(node_refs_array, item->node_refs_count, INT8OID, sizeof(int64), true, 'i'));
        slot->tts_isnull[5] = false;
    } else {
        slot->tts_values[5] = PointerGetDatum(NULL);
        slot->tts_isnull[5] = true;
    }

    if (item->members_count > 0) {
        text *members_json = cstring_to_text(encode_members(item));
        slot->tts_values[6] = PointerGetDatum(members_json);
        slot->tts_isnull[6] = false;
    } else {
        slot->tts_values[6] = PointerGetDatum(NULL);
        slot->tts_isnull[6] = true;
    }

    return ExecStoreVirtualTuple(slot);
};

void
ReScanForeignScan (ForeignScanState *node) {

    FdwExecutionState *state = node->fdw_state;
    fseek(state->file, 0, SEEK_SET);
    clear_cursor(state->cursor);
    state->cursor->position = -1;
};

void
EndForeignScan (ForeignScanState *node) {
    FdwExecutionState *state = node->fdw_state;
    free_cursor(state->cursor);
    fclose(state->file);
};

Datum
osm_fdw_handler(PG_FUNCTION_ARGS)
{
    FdwRoutine *fdwroutine = makeNode(FdwRoutine);

    fdwroutine->GetForeignRelSize = GetForeignRelSize;
    fdwroutine->GetForeignPaths = GetForeignPaths;
    fdwroutine->GetForeignPlan = GetForeignPlan;
    fdwroutine->BeginForeignScan = BeginForeignScan;
    fdwroutine->IterateForeignScan = IterateForeignScan;
    fdwroutine->ReScanForeignScan = ReScanForeignScan;
    fdwroutine->EndForeignScan = EndForeignScan;

    PG_RETURN_POINTER(fdwroutine);
}
