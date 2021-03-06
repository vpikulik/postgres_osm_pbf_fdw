
\echo Use "CREATE EXTENSION osm_fdw" to load this file. \quit

CREATE FUNCTION osm_fdw_handler()
RETURNS fdw_handler
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION osm_fdw_validator(text[], oid)
RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FOREIGN DATA WRAPPER osm_fdw
  HANDLER osm_fdw_handler
  VALIDATOR osm_fdw_validator;
