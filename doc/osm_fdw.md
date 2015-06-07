OSM PBF Foreign Data Wrapper
=======

Synopsis
--------

  Openstreetmap PBF foreign data wrapper for PostgreSQL

Description
-----------

    This extension allows to read openstreetmap PBF files directly from PostgreSQL.

Usage
-----

Small example:

```sql
CREATE EXTENSION osm_fdw;
CREATE SERVER osm_fdw_server FOREIGN DATA WRAPPER osm_fdw;

-- table defenition
CREATE FOREIGN TABLE osm_malta (
    id bigint,
    type text,
    lat double precision,
    lon double precision,
    tags jsonb,
    refs bigint[],
    members jsonb,

    version int,
    changeset bigint,
    user_id int,
    username text,
    visible boolean
)
SERVER osm_fdw_server
OPTIONS (
    filename '/path_to_file/malta-latest.osm.pbf'
);

-- find count of nodes
SELECT count(*) FROM osm_malta WHERE type='NODE';

```

Find more examples here: [examples](https://bitbucket.org/vpikulik/postgres_osm_pbf_fdw/src/d4b8a97fffd9ab5df538a78c1b2967b71a551f80/examples/ "osm fdw examples")

Table structure
---------------

The table should have the same structure as in example.
Openstreetmap has 3 main object types: NODE, WAY and RELATION.
The table row is a container that can contain all this 3 types.
Column `type` is text column and shows type of the openstreetmap object (`NODE`, `WAY`, `RELATION`)

Columns version, changeset, user_id, username, visible are informative and not empty for all objects.

List of columns for every object type:

1. Node ([OSM NODE](http://wiki.openstreetmap.org/wiki/Node "Node"))
    * id
    * type = 'NODE'
    * lat
    * lon
    * tags

1. Way ([OSM WAY](http://wiki.openstreetmap.org/wiki/Way "Way"))
    * id
    * type = 'WAY'
    * tags
    * refs (List of included nodes)

1. Relation ([OSM RELATION](http://wiki.openstreetmap.org/wiki/Relation "Relation"))
    * id
    * type = 'RELATION'
    * tags
    * members

`id` is not unique, but `(id, type)` is unique.

`tags` and `members` are JSON in v9.3 and jsonb in v9.4


Support
-------

  ([Issues trackes](https://bitbucket.org/vpikulik/postgres_osm_pbf_fdw/issues?status=new&status=open "Issues tracker"))

Author
------

    Vitali Pikulik
