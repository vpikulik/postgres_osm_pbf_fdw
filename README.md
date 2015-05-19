OSM PBF Foreign Data Wrapper
============================

Introduction
------------

This is PostgresQL foreign data wrapper that allows to read openstreetmaps *.pbf files.

Requirements
------------

    * postgres
    * libprotobuf-c
    * json-c
    * zlib

Compile
-------

I tried to cimpile only in Debian 8.

Install debian packages:
```bash
sudo apt-get install postgresql-server-dev-9.3 libjson-c-dev libjson-c2 libprotobuf-c-dev libprotobuf-c-compiler libprotobuf-c1 zlib1g-dev zlib1g
```

To compile for postgresql v9.3 run:
```bash
make
```
if you use postgresql v9.4 run:
```bash
make all94
```

Install
-------

```bash
sudo make install
```

Examples
--------

You can find example of full text address search for Berlin in examples/berlin_search.sql (or berlin_search_v94.sql)

1. Please download the latest pbf file of Berlin from http://download.geofabrik.de/europe/germany/berlin-latest.osm.pbf
1. Change path to the downloaded file in foreign table options.
1. Run in psql with supersuser permissions:

    ```
    \i examples/berlin_search.sql
    ```

1. Try to search:

    ```
    SELECT osm_link(id, type) FROM berlin_osm_search('neuendorfer strasse 1');
    ```
