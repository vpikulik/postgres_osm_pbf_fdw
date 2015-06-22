
-- Format the output for nice TAP.
\pset format unaligned
\pset tuples_only true
\pset pager
-- Revert all changes on failure.
\set ON_ERROR_ROLLBACK 1
\set ON_ERROR_STOP true
\set QUIET 1


CREATE EXTENSION IF NOT EXISTS osm_fdw;
DROP SERVER IF EXISTS osm_fdw_test_server CASCADE;
CREATE SERVER osm_fdw_test_server FOREIGN DATA WRAPPER osm_fdw;
CREATE FOREIGN TABLE osm_test_monaco (
    id bigint,
    type text,
    lat double precision,
    lon double precision,
    tags jsonb,
    refs bigint[],
    members jsonb,

    version int,
    modified timestamp,
    changeset bigint,
    user_id int,
    username text,
    visible boolean
)
SERVER osm_fdw_test_server
OPTIONS (
    filename '/tmp/monaco.osm.pbf'
);


BEGIN;

SELECT plan(11);

-- check rows count
SELECT ok(19736 = (SELECT count(*) FROM osm_test_monaco));
SELECT ok(17231 = (SELECT count(*) FROM osm_test_monaco WHERE type='NODE'));
SELECT ok(2398 = (SELECT count(*) FROM osm_test_monaco WHERE type='WAY'));
SELECT ok(107 = (SELECT count(*) FROM osm_test_monaco WHERE type='RELATION'));

-- check NODE
SELECT is((SELECT lat FROM osm_test_monaco WHERE id=1681897931 AND type='NODE')::text, '43.7370786');
SELECT is((SELECT lon FROM osm_test_monaco WHERE id=1681897931 AND type='NODE')::text, '7.4171807');
SELECT is('{"amenity": "atm", "operator": "CFM"}'::jsonb, (SELECT tags FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is((SELECT refs FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'), NULL);
SELECT is((SELECT members FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'), NULL);
SELECT is(1, (SELECT version FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));

PREPARE node_1681897931 AS 
    SELECT lat::text, lon::text, tags, refs, members, version, modified, changeset, user_id, username, visible
        FROM osm_test_monaco WHERE id=1681897931 AND type='NODE';
SELECT results_eq(
    'node_1681897931',
    $$ SELECT '43.7370786'::text, '7.4171807'::text, '{"amenity": "atm", "operator": "CFM"}'::jsonb, NULL, NULL, 1::int, '2012-03-19T17:57:31Z'::timestamp, 11032581::bigint, 629394::int, 'TorAlba'::text, true::boolean $$
);




SELECT * FROM finish();
ROLLBACK;

--DROP SERVER IF EXISTS osm_fdw_test_server CASCADE;
