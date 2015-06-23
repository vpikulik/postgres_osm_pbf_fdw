
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

SELECT plan(4 + 11 + 11 + 11);

-- check rows count
SELECT ok(19736 = (SELECT count(*) FROM osm_test_monaco));
SELECT ok(17231 = (SELECT count(*) FROM osm_test_monaco WHERE type='NODE'));
SELECT ok(2398 = (SELECT count(*) FROM osm_test_monaco WHERE type='WAY'));
SELECT ok(107 = (SELECT count(*) FROM osm_test_monaco WHERE type='RELATION'));

-- check NODE
SELECT is('43.7370786', (SELECT lat FROM osm_test_monaco WHERE id=1681897931 AND type='NODE')::text);
SELECT is('7.4171807', (SELECT lon FROM osm_test_monaco WHERE id=1681897931 AND type='NODE')::text);
SELECT is('{"amenity": "atm", "operator": "CFM"}', (SELECT tags FROM osm_test_monaco WHERE id=1681897931 AND type='NODE')::text);
SELECT is(NULL, (SELECT refs FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is(NULL, (SELECT members FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is(1, (SELECT version FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is('2012-03-19 17:57:31'::timestamp, (SELECT modified FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is(11032581::bigint, (SELECT changeset FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is(629394, (SELECT user_id FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is('TorAlba'::text, (SELECT username FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is(true, (SELECT visible FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));

-- check WAY
SELECT is(NULL, (SELECT lat FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT is(NULL, (SELECT lon FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT is(
    '{"highway": "service", "oneway": "yes"}',
    (SELECT tags FROM osm_test_monaco WHERE id=4227156 AND type='WAY')::text
);
SELECT is(
    ARRAY[25201041,25203465,1696714131,1696714132,25203467,25203470,25201047]::bigint[],
    (SELECT refs FROM osm_test_monaco WHERE id=4227156 AND type='WAY')
);
SELECT is(NULL, (SELECT members FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT is(3, (SELECT version FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT is('2012-08-23 20:36:39'::timestamp, (SELECT modified FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT is(12837776::bigint, (SELECT changeset FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT is(805403, (SELECT user_id FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT is('Luc Monaco', (SELECT username FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT is(true, (SELECT visible FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));

-- check RELATION
SELECT is(NULL, (SELECT lat FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is(NULL, (SELECT lon FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is(
    'France (terres)',
    (SELECT tags->>'name' FROM osm_test_monaco WHERE id=11980 AND type='RELATION')
);
SELECT is(NULL, (SELECT refs FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is(
    '{"role": "admin_centre", "type": "NODE", "id": 17807753}',
    (SELECT members->0 FROM osm_test_monaco WHERE id=11980 AND type='RELATION')::text
);
SELECT is(706, (SELECT version FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is('2014-10-13 12:06:07'::timestamp, (SELECT modified FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is(26046342::bigint, (SELECT changeset FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is(90780, (SELECT user_id FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is('Verdy_p', (SELECT username FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is(true, (SELECT visible FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));

SELECT * FROM finish();
ROLLBACK;

DROP SERVER IF EXISTS osm_fdw_test_server CASCADE;
