//
// Created by Patryk Mulica    on 04/05/24.
//

#ifndef DRONE8_SERVER_HPP
#define DRONE8_SERVER_HPP

#include "../con2db/pgsql.h"

#define DBNAME "drone8"

class Report {
public:
    Report(); // TODO: prenderà i dati che il server prende dal db da vedere ancora

};

class Server {
public:
    Server();

    Report createReport(int cc_id);

    void listen();

private:
    PGconn *conn;
};


#endif //DRONE8_SERVER_HPP
