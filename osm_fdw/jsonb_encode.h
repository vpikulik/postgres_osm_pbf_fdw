
#ifndef _JSONB_ENCODER_INCLUDED
#define _JSONB_ENCODER_INCLUDED

#include <string.h>

#include "postgres.h"

#include "utils/builtins.h"
#include "utils/numeric.h"
#include "utils/jsonb.h"

#include "osm_reader.h"

Datum jsonb_encode_tags(OsmItem* item);
Datum jsonb_encode_members(OsmItem* item);

#endif