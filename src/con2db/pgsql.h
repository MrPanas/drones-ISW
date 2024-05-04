//
// Created by Patryk Mulica    on 04/05/24.
//

#ifndef DRONE8_PGSQL_H
#define DRONE8_PGSQL_H

#ifndef pgsql_h
#define pgsql_h

#include <postgresql/libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

class Con2DB {

private:
  PGconn *conn;

public:
  /* connect to DB */
  Con2DB(const char *hostname, const char *port, const char *username,
         const char *password,
         const char *dbname); //  PGconn*  connect2db();

  /* use this for commands returning no data, e.g. INSERT, UPDATE */
  PGresult *ExecSQLcmd(char *sqlcmd);

  /* use this for commands returning data, e.g. SELECT */
  PGresult *ExecSQLtuples(char *sqlcmd);

  /* disconnect from DB */
  void finish();
};

#endif

#endif // DRONE8_PGSQL_H
