
--CREATE OR REPLACE LANGUAGE plpgsql;
DROP FUNCTION IF EXISTS create_osm_table(text,text);
CREATE OR REPLACE FUNCTION create_osm_table(table_name text, server_name text, file_name text) RETURNS void AS $$
BEGIN
    EXECUTE 'CREATE FOREIGN TABLE ' || quote_ident(table_name) || ' (
        id bigint,
        type text,
        lat double precision,
        lon double precision,
        tags json,
        refs bigint[],
        members json,

        version int,
        modified timestamp,
        changeset bigint,
        user_id int,
        username text,
        visible boolean
    )
    SERVER ' || quote_ident(server_name) || '
    OPTIONS (filename ' || quote_literal(file_name) || ');';
END;
$$ LANGUAGE plpgsql;
