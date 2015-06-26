# OSM PBF Foreign Data Wrapper

## Synopsis

Openstreetmap PBF foreign data wrapper for PostgreSQL

## Description

This extension for PostgreSQL implements [Foreign Data Wrapper](https://wiki.postgresql.org/wiki/Foreign_data_wrappers "Openstreetmap pbf foreign data wrapper") (FDW) for reading [Openstreetmap PBF](http://wiki.openstreetmap.org/wiki/PBF_Format "Openstreetmap PBF") file format (*.osm.pbf)

## Usage

Example:
```sql
CREATE EXTENSION osm_fdw;
CREATE SERVER osm_fdw_server FOREIGN DATA WRAPPER osm_fdw;
-- table definition
SELECT create_osm_table('osm_malta', 'osm_fdw_server', '/path_to_file/malta-latest.osm.pbf');
-- count all nodes
SELECT count(*) FROM osm_malta WHERE type='NODE';
```

Find more examples here: [examples](https://github.com/vpikulik/postgres_osm_pbf_fdw/tree/master/examples "Openstreetmap foreign data wrapper examples")

## How to use

Create extension at first:
```sql
CREATE EXTENSION osm_fdw;
```

To access foreign data, you need to create a [foreign server object](http://www.postgresql.org/docs/9.4/static/ddl-foreign-data.html "Postgresql foreign server object"):
```sql
CREATE SERVER osm_fdw_server FOREIGN DATA WRAPPER osm_fdw;
```

Next you should create [foreign table](http://www.postgresql.org/docs/9.4/static/sql-createforeigntable.html). 
There are two ways: `CREATE FOREIGN TABLE` query and `create_osm_table` function.
I strongly recommend to use second method.
The function `create_osm_table(text, text, text)` is provided together with the extension.
It requires 3 parameters: a name of the table, a name of the foreign server object and a path to *.osm.pbf file.
```sql
SELECT create_osm_table('table_name', 'osm_fdw_server', '/path_to_file/file.osm.pbf');
```

FDW start to read the file with every query.
I would recommend to create [materialized view](http://www.postgresql.org/docs/9.4/static/rules-materializedviews.html "Postgresql materialized view") to fast data access.
```sql
CREATE MATERIALIZED VIEW osm_data AS SELECT * FROM osm_foreign_table WITH DATA;
```

## Table structure

Table should have this structure:
```sql
CREATE FOREIGN TABLE table_name (
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
SERVER osm_fdw_server
OPTIONS (
    filename '/path_to_file/file.osm.pbf'
);
```
Fields can have other names, but position and types must be as in this example.

### Column types
The FDW can read 3 openstreetmap types: ([NODE](http://wiki.openstreetmap.org/wiki/Node "Node")), ([WAY](http://wiki.openstreetmap.org/wiki/Way "Way")) and ([RELATION](http://wiki.openstreetmap.org/wiki/Relation "Relation")).

    * `id` - OSM object id
    * `type` - type of the object (Values: NODE, WAY, RELATION)
    * `lat` - latitude (not empty only for NODE)
    * `lon` - longitude (not empty only for NODE type)
    * `tags` - json object with OSM tags (json for postgres 9.3 and jsonb for 9.4)
    * `refs` - array on node ids (not empty only for WAY)
    * `members` - array of objects with relation members (not empty only for RELATION; json for postgres 9.3 and jsonb for 9.4)
    * `version` - OSM version
    * `modified` - OSM last change date
    * `changeset` - OSM changeset
    * `user_id` - id of the OSM user
    * `username` - name of the OSM user
    * `visible` - shows if object is visible
