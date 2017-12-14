
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
SELECT create_osm_table('osm_test_monaco', 'osm_fdw_test_server', '/tmp/monaco.osm.pbf');


DROP FUNCTION IF EXISTS eq_json(json, json);
CREATE FUNCTION eq_json(jdata1 json, jdata2 json) RETURNS boolean AS $$
DECLARE
    keys1 text[];
    keys2 text[];
    values1 text[];
    values2 text[];
BEGIN
    SELECT array_agg(key) INTO keys1 FROM json_each_text(jdata1);
    SELECT array_agg(key) INTO keys2 FROM json_each_text(jdata2);
    SELECT array_agg(value) INTO values1 FROM json_each_text(jdata1);
    SELECT array_agg(value) INTO values2 FROM json_each_text(jdata2);
    RETURN keys1 = keys2 AND values1 = values2;
END;
$$ LANGUAGE plpgsql;


BEGIN;

SELECT plan(4 + 11 + 11 + 11);

-- check rows count
SELECT ok(19739 = (SELECT count(*) FROM osm_test_monaco));
SELECT ok(17233 = (SELECT count(*) FROM osm_test_monaco WHERE type='NODE'));
SELECT ok(2398 = (SELECT count(*) FROM osm_test_monaco WHERE type='WAY'));
SELECT ok(108 = (SELECT count(*) FROM osm_test_monaco WHERE type='RELATION'));

-- check NODE
SELECT is('43.7370786', (SELECT lat FROM osm_test_monaco WHERE id=1681897931 AND type='NODE')::text);
SELECT is('7.4171807', (SELECT lon FROM osm_test_monaco WHERE id=1681897931 AND type='NODE')::text);
SELECT ok(eq_json(
    '{"amenity": "atm", "operator": "CFM"}'::json,
    (SELECT tags FROM osm_test_monaco WHERE id=1681897931 AND type='NODE')::json
));
-- SELECT is('{"amenity": "atm", "operator": "CFM"}', (SELECT tags FROM osm_test_monaco WHERE id=1681897931 AND type='NODE')::text);
SELECT is(NULL, (SELECT refs FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT ok((SELECT members FROM osm_test_monaco WHERE id=1681897931 AND type='NODE') is NULL);
SELECT is(1, (SELECT version FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is('2012-03-19 17:57:31'::timestamp, (SELECT modified FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is(11032581::bigint, (SELECT changeset FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is(629394, (SELECT user_id FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is('TorAlba'::text, (SELECT username FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));
SELECT is(true, (SELECT visible FROM osm_test_monaco WHERE id=1681897931 AND type='NODE'));

-- check WAY
SELECT is(NULL, (SELECT lat FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT is(NULL, (SELECT lon FROM osm_test_monaco WHERE id=4227156 AND type='WAY'));
SELECT ok(eq_json(
    '{"highway": "service", "oneway": "yes"}'::json,
    (SELECT tags FROM osm_test_monaco WHERE id=4227156 AND type='WAY')::json
));
SELECT is(
    ARRAY[25201041,25203465,1696714131,1696714132,25203467,25203470,25201047]::bigint[],
    (SELECT refs FROM osm_test_monaco WHERE id=4227156 AND type='WAY')
);
SELECT ok((SELECT members FROM osm_test_monaco WHERE id=4227156 AND type='WAY') is NULL);
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
SELECT ok(eq_json(
    '{"role": "admin_centre", "type": "NODE", "id": 17807753}'::json,
    (SELECT members->0 FROM osm_test_monaco WHERE id=11980 AND type='RELATION')::json
));
SELECT is(706, (SELECT version FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is('2014-10-13 12:06:07'::timestamp, (SELECT modified FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is(26046342::bigint, (SELECT changeset FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is(90780, (SELECT user_id FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is('Verdy_p', (SELECT username FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));
SELECT is(true, (SELECT visible FROM osm_test_monaco WHERE id=11980 AND type='RELATION'));

SELECT * FROM finish();
ROLLBACK;

DROP SERVER IF EXISTS osm_fdw_test_server CASCADE;
