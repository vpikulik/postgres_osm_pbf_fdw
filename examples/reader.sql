
CREATE EXTENSION IF NOT EXISTS osm_fdw;

DROP FOREIGN TABLE IF EXISTS osm_malta;
DROP FOREIGN TABLE IF EXISTS osm_monaco;
DROP FOREIGN TABLE IF EXISTS osm_belarus;
DROP SERVER IF EXISTS osm_fdw_server;

CREATE SERVER osm_fdw_server FOREIGN DATA WRAPPER osm_fdw;
SELECT create_osm_table('osm_malta', 'osm_fdw_server', '/home/promo/Downloads/malta-latest.osm.pbf');
SELECT create_osm_table('osm_monaco', 'osm_fdw_server', '/home/promo/Downloads/monaco-latest.osm.pbf');
SELECT create_osm_table('osm_belarus', 'osm_fdw_server', '/home/promo/Downloads/belarus-latest.osm.pbf');
