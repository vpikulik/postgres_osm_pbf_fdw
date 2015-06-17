OSM PBF Foreign Data Wrapper
============================

Introduction
------------

This is PostgreSQL foreign data wrapper that allows to read openstreetmaps *.pbf files.

Requirements
------------

    * postgresql version 9.3 or 9.4
    * libprotobuf-c
    * json-c
    * zlib

Install
-------

Install requirements:
```bash
sudo apt-get install postgresql-server-dev libjson-c-dev libjson-c2 libprotobuf-c-dev libprotobuf-c-compiler libprotobuf-c1 zlib1g-dev zlib1g
```

Compile extension and install:
```bash
sudo make install
```

`pg_config` can be specified:
```bash
sudo make PG_CONFIG=/usr/lib/postgresql/9.4/bin/pg_config install
```

Check [examples](https://github.com/vpikulik/postgres_osm_pbf_fdw/tree/master/examples "osm fdw examples")
