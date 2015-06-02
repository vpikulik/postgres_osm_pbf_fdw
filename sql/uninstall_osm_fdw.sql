/*
 * Author: The maintainer's name
 * Created at: 2015-06-02 09:43:22 +0200
 *
 */

--
-- This is a example code genereted automaticaly
-- by pgxn-utils.

SET client_min_messages = warning;

BEGIN;

-- You can use this statements as
-- template for your extension.

DROP OPERATOR #? (text, text);
DROP FUNCTION osm_fdw(text, text);
DROP TYPE osm_fdw CASCADE;
COMMIT;
