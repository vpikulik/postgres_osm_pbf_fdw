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
make
sudo make install
```

Check [examples](https://bitbucket.org/vpikulik/postgres_osm_pbf_fdw/src/c45bc4c5b209e6d5ef1454e010b687f67445f7bc/examples/?at=master "osm fdw examples")
