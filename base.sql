
CREATE EXTENSION IF NOT EXISTS osm_fdw;

DROP FOREIGN TABLE IF EXISTS osm_source;
DROP SERVER IF EXISTS osm_fdw_server;

CREATE SERVER osm_fdw_server FOREIGN DATA WRAPPER osm_fdw;
CREATE FOREIGN TABLE osm_source (id integer, type text, lat double precision, lon double precision, tags json) SERVER osm_fdw_server;
