
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
    int cursor_index;
    int cursor_position;
    Cursor* cursor;
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
BeginForeignScan (ForeignScanState *node,
                  int eflags) {

    FdwExecutionState *state = (FdwExecutionState*) palloc(sizeof(FdwExecutionState));

    state->file = fopen("/home/promo/Downloads/belarus-latest.osm.pbf", "r");
    state->cursor_index = 0;
    state->cursor_position = 0;

    node->fdw_state = (void *) state;
};

TupleTableSlot *
IterateForeignScan (ForeignScanState *node){

    FdwExecutionState *state = node->fdw_state;

    if (feof(state->file) || state->cursor_index > 3) {
        return NULL;
    }

    TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
    ExecClearTuple(slot);

    if (state->cursor_position == 0) {
        state->cursor = init_cursor();
        fill_cursor(state->cursor, state->file, state->cursor_index == 0);
    }

    OsmNode* osm_node = state->cursor->nodes[state->cursor_position];

    // columns: id, type, lat, lon, tags

    int64 p_id = (int64)osm_node->id;
    slot->tts_values[0] = Int64GetDatum(p_id);
    slot->tts_isnull[0] = false;

    text *type_name = cstring_to_text("Node");
    slot->tts_values[1] = PointerGetDatum(type_name);
    slot->tts_isnull[1] = false;

    float8 p_lat = (float8)osm_node->lat;
    slot->tts_values[2] = Float8GetDatum(p_lat);
    slot->tts_isnull[2] = false;

    float8 p_lon = (float8)osm_node->lon;
    slot->tts_values[3] = Float8GetDatum(p_lon);
    slot->tts_isnull[3] = false;

    if (osm_node->tags_count > 0) {
        json_object* jtags = encode_tags(osm_node);
        text *tags_json = cstring_to_text(encode_json(jtags));
        slot->tts_values[4] = PointerGetDatum(tags_json);
        slot->tts_isnull[4] = false;
    } else {
        slot->tts_values[4] = PointerGetDatum(NULL);
        slot->tts_isnull[4] = true;
    }


    state->cursor_position += 1;
    if (state->cursor_position >= state->cursor->nodes_count) {
        state->cursor_position = 0;
        state->cursor_index += 1;
        free_cursor(state->cursor);
    }

    return ExecStoreVirtualTuple(slot);
};

void
ReScanForeignScan (ForeignScanState *node) {

    FdwExecutionState *state = node->fdw_state;
    state->cursor_position = 0;
    state->cursor_index = 0;
    fseek(state->file, 0, SEEK_SET);
    if (state->cursor) free_cursor(state->cursor);
};

void
EndForeignScan (ForeignScanState *node) {
    FdwExecutionState *state = node->fdw_state;
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
