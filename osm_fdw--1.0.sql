/* contrib/osm_fdw/osm_fdw--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION osm_fdw" to load this file. \quit

CREATE FUNCTION osm_fdw_handler()
RETURNS fdw_handler
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FOREIGN DATA WRAPPER osm_fdw
  HANDLER osm_fdw_handler;