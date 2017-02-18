
DROP MATERIALIZED VIEW IF EXISTS berlin_osm_data;
DROP FOREIGN TABLE IF EXISTS osm_berlin;

CREATE EXTENSION IF NOT EXISTS osm_fdw;
DROP SERVER IF EXISTS osm_fdw_test_server CASCADE;
CREATE SERVER osm_fdw_server FOREIGN DATA WRAPPER osm_fdw;

SELECT create_osm_table('osm_berlin', 'osm_fdw_server', '/home/promo/berlin-latest.osm.pbf');

CREATE MATERIALIZED VIEW berlin_osm_data AS
    SELECT id, type, lat, lon, tags
        FROM osm_berlin WHERE tags->>'addr:postcode' is not NULL
    WITH DATA;


CREATE OR REPLACE FUNCTION text_index_from_json(data jsonb) RETURNS text AS $$
    SELECT string_agg(value->>0, ' ')
        FROM jsonb_each(data)
        WHERE key LIKE 'name%' OR key LIKE 'addr:%';
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION berlin_osm_search(text_query text) RETURNS TABLE(id bigint, type text) AS $$
    SELECT id, type
        FROM berlin_osm_data
        WHERE to_tsvector('german', text_index_from_json(tags)) @@ to_tsquery('german', text_query);
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION osm_link(id bigint, osm_type text) RETURNS text AS $$
    SELECT 'http://www.openstreetmap.org/' || lower(osm_type) || '/' || id || '/';
$$ LANGUAGE SQL IMMUTABLE;

CREATE OR REPLACE FUNCTION osm_xml_link(id bigint, osm_type text) RETURNS text AS $$
    SELECT 'http://www.openstreetmap.org/api/0.6/' || lower(osm_type) || '/' || id || '/';
$$ LANGUAGE SQL IMMUTABLE;

CREATE INDEX berlin_osm_ft_index
    ON berlin_osm_data
    USING gin(to_tsvector('german', text_index_from_json(tags)));


-- SELECT osm_link(id, type) FROM berlin_osm_search('mohrenstrasse');
