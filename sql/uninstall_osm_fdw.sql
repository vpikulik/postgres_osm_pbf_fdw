
DROP FOREIGN DATA WRAPPER IF EXISTS osm_fdw CASCADE;

DROP FUNCTION IF EXISTS osm_fdw_handler();
DROP FUNCTION IF EXISTS osm_fdw_validator(text[], oid);
