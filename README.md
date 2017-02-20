# OSM PBF Foreign Data Wrapper

## Introduction

This extension for PostgreSQL implements [Foreign Data Wrapper](https://wiki.postgresql.org/wiki/Foreign_data_wrappers "Openstreetmap pbf foreign data wrapper") (FDW) for reading [Openstreetmap PBF](http://wiki.openstreetmap.org/wiki/PBF_Format "Openstreetmap PBF") file format (*.osm.pbf)

## Requirements

    * postgresql version 9.6
    * libprotobuf-c
    * json-c >= 0.11
    * zlib

## Install

### How to install in Debian 8

```bash
sudo apt-get install postgresql-server-dev libjson-c-dev libjson-c2 libprotobuf-c-dev libprotobuf-c-compiler libprotobuf-c1 zlib1g-dev zlib1g
sudo make install
```

### Install with PGXN

Install required packages before and next run:
```bash
sudo pgxn install osm_fdw
```

### Addition parameters
If you want to use not default version of postgres in your system, you can specify `pg_config`
```bash
sudo make PG_CONFIG=/usr/lib/postgresql/9.6/bin/pg_config install
```

## Documentation
Find more documentation here: [osm_fdw doc](https://github.com/vpikulik/postgres_osm_pbf_fdw/blob/master/doc/osm_fdw.md "Openstreetmap foreign data wrapper documentation")

## Examples
Find more examples here: [examples](https://github.com/vpikulik/postgres_osm_pbf_fdw/tree/master/examples "Openstreetmap foreign data wrapper examples")

## Support
([Issues trackes](https://github.com/vpikulik/postgres_osm_pbf_fdw/issues "Issues tracker"))
Email: v.pikulik@gmail.com

## Author
Vitali Pikulik <v.pikulik@gmail.com>
