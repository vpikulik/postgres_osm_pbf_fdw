
CREATE EXTENSION IF NOT EXISTS osm_fdw;

DROP FOREIGN TABLE IF EXISTS osm_malta;
DROP FOREIGN TABLE IF EXISTS osm_monaco;
DROP FOREIGN TABLE IF EXISTS osm_belarus;
DROP SERVER IF EXISTS osm_fdw_server;

CREATE SERVER osm_fdw_server FOREIGN DATA WRAPPER osm_fdw;
CREATE FOREIGN TABLE osm_malta (
    id bigint,
    type text,
    lat double precision,
    lon double precision,
    tags json,
    refs bigint[],
    members json,

    version int,
    changeset bigint,
    user_id int,
    username text,
    visible boolean
)
SERVER osm_fdw_server
OPTIONS (
    filename '/home/promo/Downloads/malta-latest.osm.pbf'
);
CREATE FOREIGN TABLE osm_monaco (
    id bigint,
    type text,
    lat double precision,
    lon double precision,
    tags json,
    refs bigint[],
    members json,

    version int,
    changeset bigint,
    user_id int,
    username text,
    visible boolean
)
SERVER osm_fdw_server
OPTIONS (
    filename '/home/promo/Downloads/monaco-latest.osm.pbf'
);
CREATE FOREIGN TABLE osm_belarus (
    id bigint,
    type text,
    lat double precision,
    lon double precision,
    tags json,
    refs bigint[],
    members json,

    version int,
    changeset bigint,
    user_id int,
    username text,
    visible boolean
)
SERVER osm_fdw_server
OPTIONS (
    filename '/home/promo/Downloads/belarus-latest.osm.pbf'
);
