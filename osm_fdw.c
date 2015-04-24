
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

    slot->tts_values[0] = PointerGetDatum(NULL);
    slot->tts_isnull[0] = false;

    slot->tts_values[1] = PointerGetDatum(NULL);
    slot->tts_isnull[1] = false;

    slot->tts_values[2] = PointerGetDatum(NULL);
    slot->tts_isnull[2] = false;

    slot->tts_values[3] = PointerGetDatum(NULL);
    slot->tts_isnull[3] = false;

    slot->tts_values[4] = PointerGetDatum(NULL);
    slot->tts_isnull[4] = false;


    state->cursor_position += 1;
    if (state->cursor_position >= state->cursor->nodes_count) {
        state->cursor_position = 0;
        free_cursor(state->cursor);
    }

    return ExecStoreVirtualTuple(slot);


    // // 0 columnt (P:Int4 - C:Int32)
    // int32 value_0 = (int32) state->current_index + 10;
    // slot->tts_values[0] = Int32GetDatum(value_0);
    // slot->tts_isnull[0] = false;

    // // 1 column (P:text - C:text*)
    // text *value_1 = cstring_to_text("abcdefg");
    // slot->tts_values[1] = PointerGetDatum(value_1);
    // slot->tts_isnull[1] = false;

    // // 2 column (Int32 array)
    // int values_count = state->current_index;
    // int i;
    // Datum *values_2 = (Datum*) palloc(sizeof(Datum)*values_count);
    // for (i=0;i<values_count;i++) {
    //     int32 array_item = i*i;
    //     values_2[i] = Int32GetDatum(array_item);
    // }
    // ArrayType *value_2 = construct_array(values_2, values_count, INT4OID, sizeof(int32), true, 'i');
    // slot->tts_values[2] = PointerGetDatum(value_2);
    // slot->tts_isnull[2] = false;

    // // 3 column (text array)
    // Datum *values_3 = (Datum*) palloc(sizeof(Datum)*values_count);
    // for (i=0;i<values_count;i++) {
    //     text *array_item = cstring_to_text("abcdefg");
    //     values_3[i] = PointerGetDatum(array_item);
    // }
    // ArrayType *value_3 = construct_array(values_3, values_count, TEXTOID, -1, true, 'i');
    // slot->tts_values[3] = PointerGetDatum(value_3);
    // slot->tts_isnull[3] = false;

    // // 4 column (json)
    // text *value_4 = cstring_to_text("{\"a\": \"b\"}");
    // slot->tts_values[4] = PointerGetDatum(value_4);
    // slot->tts_isnull[4] = false;

    // state->current_index += 1;
    
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
