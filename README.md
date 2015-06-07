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

Compile
-------

Install debian packages:
```bash
sudo apt-get install postgresql-server-dev libjson-c-dev libjson-c2 libprotobuf-c-dev libprotobuf-c-compiler libprotobuf-c1 zlib1g-dev zlib1g
```

Run:
```bash
make
```

Install
-------

```bash
sudo make install
```
