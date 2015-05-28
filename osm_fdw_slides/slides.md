% title: Postgres + Openstreetmaps
% subtitle: OSM foreign data wrapper 
% author: Vitaly Pikulik
% author:
% thankyou: Thanks everyone!
% thankyou_details: And especially these people:
% contact:
% contact:
% favicon: http://www.stanford.edu/favicon.ico

---
title: Openstreetmap

Types:

- Node (coordinates, tags)
- Way (refs, tags)
- Relation (members, tags)

---
title: Openstreetmap file formats

Formats:

- OSM (XML + GZIP/BZIP)
- PBF
    * 30% smaller
    * 5-6 time faster to r/w

---
title: PBF (Protocol buffers)

What are protocol buffers?

Protocol buffers are Google's language-neutral, platform-neutral, extensible mechanism for serializing structured data – think XML, but smaller, faster, and simpler.

You define how you want your data to be structured once, then you can use special generated source code to easily write and read your structured data to and from a variety of data streams and using a variety of languages.

---
title: Postgres foreign data wrapper

PostgreSQL allows to access data that resides outside database with regular SQL queries.

Foreign data an be accessed with the help from a *foreign data wrapper*.

---
title: Create foreign table
class: nobackground

<pre class="prettyprint" data-lang="sql">

CREATE EXTENSION IF NOT EXISTS osm_fdw;
CREATE SERVER osm_fdw_server FOREIGN DATA WRAPPER osm_fdw;

CREATE FOREIGN TABLE osm_berlin (
    id bigint, type text, 
    lat double precision, lon double precision,
    tags jsonb, refs bigint[], members jsonb,

    version int, changeset bigint, user_id int, username text, visible boolean
)
SERVER osm_fdw_server
OPTIONS (
    filename '/path/berlin-latest.osm.pbf'
);

</pre>

---
title: Street search

Prepare index:

<pre class="prettyprint" data-lang="sql">
    CREATE MATERIALIZED VIEW berlin_osm_data AS
        SELECT id, type, lat, lon, tags, refs, members 
            FROM osm_berlin WHERE tags IS NOT NULL WITH DATA;

    CREATE OR REPLACE FUNCTION text_index_from_json(data jsonb) RETURNS text AS $$
        SELECT string_agg(value, ' ')
        FROM (SELECT key, value FROM jsonb_each_text(data)) as data_keys;
    $$ LANGUAGE SQL IMMUTABLE;

    CREATE INDEX berlin_osm_ft_index ON berlin_osm_data
        USING gin(to_tsvector('german', text_index_from_json(tags)));
</pre>

---
title: Street search

Search:

<pre class="prettyprint" data-lang="sql">
    CREATE OR REPLACE FUNCTION berlin_osm_search(text_query text)
     RETURNS TABLE(id bigint, type text) AS $$
        SELECT id, type FROM berlin_osm_data
        WHERE to_tsvector('german', text_index_from_json(tags)) @@
         plainto_tsquery('german', text_query);
    $$ LANGUAGE SQL IMMUTABLE;

    SELECT type, id FROM berlin_osm_search('mohrenstrasse 60');
</pre>
