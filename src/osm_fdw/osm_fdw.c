
#include "postgres.h"
#include "catalog/pg_type.h"
#include "commands/defrem.h" // defGetString
#include "foreign/fdwapi.h"
#include "foreign/foreign.h"
#include "optimizer/cost.h"
#include "optimizer/pathnode.h" // create_foreignscan_path
#include "optimizer/planmain.h" // make_foreignscan
#include "optimizer/restrictinfo.h" // extract_actual_clauses
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/rel.h" // RelationGetRelid
#include "utils/timestamp.h"
#include "access/reloptions.h" // untransformRelOptions
#include "catalog/pg_foreign_table.h" // ForeignTableRelationId

PG_MODULE_MAGIC;


#include "osm_reader.h"
#ifdef USE_LIBJSONC
#include "utils/json.h"
#include "json_encode.h"
#endif
#ifdef USE_JSONB
#include "utils/jsonb.h"
#include "jsonb_encode.h"
#endif


typedef struct FdwExecutionState
{
    FILE* file;
    Cursor* cursor;
    file_size_t file_size;
} FdwExecutionState;


char* get_file_name(Oid foreigntableid) {
    ForeignTable *table = GetForeignTable(foreigntableid);
    List *options;
    ListCell *lc, *prev;
    options = NIL;
    options = list_concat(options, table->options);
    char *filename = NULL;
    prev = NULL;
    foreach(lc, options) {
        DefElem *def = (DefElem *) lfirst(lc);

        if (strcmp(def->defname, "filename") == 0)
        {
            filename = defGetString(def);
            options = list_delete_cell(options, lc, prev);
            pfree(def);
            break;
        }
        prev = lc;
    }

    if (filename == NULL) {
        elog(ERROR, "filename is required for file_fdw foreign tables");
    }

    return filename;
}


void
GetForeignRelSize (PlannerInfo *root,
                   RelOptInfo *baserel,
                   Oid foreigntableid) {

    char* filename = get_file_name(foreigntableid);
    FILE *file = fopen(filename, "r");

    fseek(file, 0, SEEK_END);
    file_size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    baserel->rows = estimate_items_count(file, file_size);

    fclose(file);
};

void
GetForeignPaths (PlannerInfo *root,
                 RelOptInfo *baserel,
                 Oid foreigntableid) {

    Cost startup_cost = baserel->baserestrictcost.startup;
    Cost run_cost = 0;
    Cost cpu_per_tuple = cpu_tuple_cost * 10 + baserel->baserestrictcost.per_tuple;

    // run_cost += seq_page_cost * pages;
    run_cost += cpu_per_tuple * baserel->rows;

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
    char* version = PG_VERSION;
    FdwExecutionState *state = (FdwExecutionState*) palloc(sizeof(FdwExecutionState));

    char* filename = get_file_name(RelationGetRelid(node->ss.ss_currentRelation));
    FILE *file = fopen(filename, "r");
    // free(filename);
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
        #ifdef USE_LIBJSONC
        char *tags_json = encode_tags(item);
        text *tags_text = cstring_to_text(tags_json);
        slot->tts_values[4] = PointerGetDatum(tags_text);
        slot->tts_isnull[4] = false;
        free(tags_json);
        #endif

        #ifdef USE_JSONB
        Jsonb *jtags = jsonb_encode_tags(item);
        slot->tts_values[4] = PointerGetDatum(jtags);
        slot->tts_isnull[4] = false;
        #endif
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
        ArrayType *node_refs_array_values = construct_array(node_refs_array, item->node_refs_count, INT8OID, sizeof(int64), true, 'i');
        slot->tts_values[5] = PointerGetDatum(node_refs_array_values);
        slot->tts_isnull[5] = false;
        pfree(node_refs_array);
    } else {
        slot->tts_values[5] = PointerGetDatum(NULL);
        slot->tts_isnull[5] = true;
    }

    if (item->members_count > 0) {
        #ifdef USE_LIBJSONC
        char *members_json = encode_members(item);
        text *members_text = cstring_to_text(members_json);
        slot->tts_values[6] = PointerGetDatum(members_text);
        slot->tts_isnull[6] = false;
        free(members_json);
        #endif

        #ifdef USE_JSONB
        Jsonb *jmembers = jsonb_encode_members(item);
        slot->tts_values[6] = PointerGetDatum(jmembers);
        slot->tts_isnull[6] = false;
        #endif
    } else {
        slot->tts_values[6] = PointerGetDatum(NULL);
        slot->tts_isnull[6] = true;
    }

    int32 version = (int32)item->version;
    slot->tts_values[7] = Int32GetDatum(version);
    slot->tts_isnull[7] = false;

    pg_time_t changed_time = (pg_time_t)item->timestamp;
    TimestampTz changed = time_t_to_timestamptz(changed_time);
    slot->tts_values[8] = TimestampGetDatum(changed);
    slot->tts_isnull[8] = false;

    int64 changeset = (int64)item->changeset;
    slot->tts_values[9] = Int64GetDatum(changeset);
    slot->tts_isnull[9] = false;

    int32 uid = (int32)item->uid;
    slot->tts_values[10] = Int32GetDatum(uid);
    slot->tts_isnull[10] = false;

    text *username = cstring_to_text(item->user);
    slot->tts_values[11] = PointerGetDatum(username);
    slot->tts_isnull[11] = false;

    slot->tts_values[12] = BoolGetDatum(item->visible);
    slot->tts_isnull[12] = false;

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

static short int file_exists(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}

PG_FUNCTION_INFO_V1(osm_fdw_validator);

Datum
osm_fdw_validator(PG_FUNCTION_ARGS)
{
    Oid catalog = PG_GETARG_OID(1);
    if (catalog != ForeignTableRelationId) {
        PG_RETURN_VOID();
    }

    List *options_list = untransformRelOptions(PG_GETARG_DATUM(0));
    char *filename = NULL;
    ListCell *cell;

    foreach(cell, options_list)
    {
        DefElem *def = (DefElem *) lfirst(cell);

        if (strcmp(def->defname, "filename") == 0)
        {
            if (filename) {
                ereport(ERROR,
                        (errcode(ERRCODE_SYNTAX_ERROR),
                         errmsg("conflicting or redundant options")));
            }
            filename = defGetString(def);
        }
    }

    if (filename == NULL) {
        ereport(ERROR,
                (errcode(ERRCODE_FDW_DYNAMIC_PARAMETER_VALUE_NEEDED),
                 errmsg("filename is required for file_fdw foreign tables")));
    }

    if (!file_exists(filename)) {
        ereport(ERROR,
                (errcode(ERRCODE_FDW_ERROR),
                 errmsg("file can not be opened")));
    }

    PG_RETURN_VOID();
}