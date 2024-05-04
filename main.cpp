#include <iostream>
#include "src/control_center/ControlCenter.hpp"

#include "src/area/Area.hpp"
#include "src/redis/StreamRedis.h"
#include <hiredis/hiredis.h>
#include <libpq-fe.h>
#include <nlohmann/json.hpp>
#include "src/redis/redis.h"


// Some constants
#define DBNAME "postgres"
#define DBUSER "postgres"
#define DBPASS "postgres"
#define DBHOST "localhost"
#define DBPORT "5432"




// Establish connection to redis server
using namespace std;

using json = nlohmann::json;

using GG = std::vector<std::map<std::string,std::vector<std::string>>>;


int main() {
    cout << "Hello, World!" << endl;

    StreamRedis streamRedis{};



    cout << "VAR1: " << Var << endl;

    GG gg;

//
    Message message;
    message["ao"] = "eo";
//
//    // Send Message
//    string send = sendMessage(streamRedis.context_, "stream1", message);
//    cout << "Send Message: " << send << endl;

    // Read message from the group

    // Read message from the group

    GroupInfo info = getInfoGroup(streamRedis.context_, "stream1", "group1");
//    int temp = 0;
//    using  Response =  tuple<string,map<string,string>>;
//    while(temp <30000) {
//        cout << temp << endl;
//        temp++;
//        Response response = readMessageGroup(streamRedis.context_, "group1", "consumer1", "stream1", 0);
//        string msgId = get<0>(response);
//        if (msgId.empty()) {
////            cout << "No reply" << endl;
//            continue;
//        }
//        cout << "msgId: " << msgId << endl;
//        map<string, string> messageMap = get<1>(response);
//        // print messageMap
//        for (const auto &item : messageMap) {
//            cout << item.first << " " << item.second << endl;
//        }
//
//        // Acknowledge the message
//        long result = ackMessage(streamRedis.context_, "stream1", "group1", msgId);
//        cout << "Acknowledge: " << result << endl;
//
//        // Print the group info
//        printInfoGroup(streamRedis.context_, "stream1");
//    }





//    string id = get<0>(response);
//    cout << "Id: " << id << endl;
//    map<string, string> messageMap = get<1>(response);
//
//    for (const auto &item : messageMap) {
//        cout << item.first << " " << item.second << endl;
//    }





    return EXIT_SUCCESS;
    // Call a cc with id 1
    string ccId = "1";
    string ccCommand = "./cc " + ccId;

    int res = system(ccCommand.c_str());
    if (res != 0) {
        cout << "Error calling cc" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;


    // Create SimpleStrategy object


    // Create ControlCenter object
//    ControlCenter cc{1, &simpleStrategy};
//
//    cc.startOperations();
//
//    std::this_thread::sleep_for(std::chrono::seconds(5));
//    cc.stopOperations();


    // Create ControlCenter object



//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    cc.stopOperations();

//    cc.handleTimeout(std::chrono::seconds(5));
//    cc.stopOperation();


    // Create Drone object


    // Add drone to control center


    // Execute scan
//    controlCenter.executeScan();

    // Connect to postgresql server
//    PGconn *conn = PQsetdbLogin(DBHOST, DBPORT, nullptr, nullptr, DBNAME, DBUSER, DBPASS);
//
//    if (PQstatus(conn) != CONNECTION_OK) {
//        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
//        return EXIT_FAILURE;
//    }
//    std::cout << "Successfully connected to database" << std::endl;
//
//    // Disconnect from postgresql server
//    PQfinish(conn);



    // Create RedisStream object
//    StreamRedis streamRedis{};

    // Print the stream
//    std::cout << "Printing the first stream" << std::endl;
//    streamRedis.printStream("stream1");

//    std::cout << "Sending a message to the stream" << std::endl;

//    streamRedis.sendMessage("stream1", message);

    // print the stream
//    std::cout << "Printing the second stream" << std::endl;
//    streamRedis.printStream("stream1");
    // Create Drone message
//    DroneMessage droneMessage{};
//    int res = streamRedis.getDroneMessage("stream1", "1", droneMessage);
//    // print res
//    std::cout << "Res: " << res << std::endl;
//
//    // print droneMessage
//    std::cout << "DroneMessage: " << droneMessage.id << " " << droneMessage.status << " " << droneMessage.battery << std::endl;

    // Create Json object
//    json jsonValue;
//    jsonValue["id"] = 1;
//    jsonValue["status"] = "OK";
//    jsonValue["battery"] = 100;
//
//    streamRedis.sendJsonMessage("stream1", jsonValue);
//
//    streamRedis.printStream("stream1");

    // Create Json object with a vector
//    json jsonObject;

//    jsonValue["id"] = 1;
//    jsonValue["path"] = {"(0,0)", "(0,1)", "(0,2)", "(0,3)", "(0,4)", "(0,5)", "(0,6)", "(0,7)", "(0,8)", "(0,9)"};

//    streamRedis.sendJsonMessage("stream1", jsonValue);

//    streamRedis.printStream("stream1");










    return EXIT_SUCCESS;
}
