/*
 * Author: The maintainer's name
 * Created at: 2015-06-02 09:43:22 +0200
 *
 */

--
-- This is a example code genereted automaticaly
-- by pgxn-utils.

SET client_min_messages = warning;

-- If your extension will create a type you can
-- do somenthing like this
CREATE TYPE osm_fdw AS ( a text, b text );

-- Maybe you want to create some function, so you can use
-- this as an example
CREATE OR REPLACE FUNCTION osm_fdw (text, text)
RETURNS osm_fdw LANGUAGE SQL AS 'SELECT ROW($1, $2)::osm_fdw';

-- Sometimes it is common to use special operators to
-- work with your new created type, you can create
-- one like the command bellow if it is applicable
-- to your case

CREATE OPERATOR #? (
	LEFTARG   = text,
	RIGHTARG  = text,
	PROCEDURE = osm_fdw
);
