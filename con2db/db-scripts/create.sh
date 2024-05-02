#!/bin/sh

# This defines parameters
# parameters.sql 

# This creates db and user
# create-db-user.sql

# This creates db schema (tables)
# schema.sql

# Grant privileges to user
# grant.sql

# Run psql commands as the postgres user
# Run psql commands as the postgres user
su - postgres -c "psql postgres -f /data/db-scripts/parameters.sql -f /data/db-scripts/create-db-user.sql -f /data/db-scripts/schema.sql -f /data/db-scripts/grant.sql"
