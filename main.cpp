#include <iostream>
#include <cstdlib> // For atoi()
#include <getopt.h> // For getopt_long
#include "src/control_center/ControlCenter.hpp"
#include "src/scanning_strategy/BasicStrategy.h"
#include <atomic>
#include <csignal>

using namespace std;






void printHelp(char* name) {
    cout << "Usage: "<< name << endl;
    cout << "Options:" << endl;
    cout << "  --height <height>  Height of the area. Must be greater than 0." << endl;
    cout << "  --width <width>    Width of the area. Must be greater than 0." << endl;
}

/**
 * This main takes the following arguments:
 * --height: Height of the area
 * --width: Width of the area
 */



int main(int argc, char* argv[]) {
//    redisContext *context = redisConnect(REDIS_HOST, REDIS_PORT);
//    if (context == NULL || context->err) {
//        if (context) {
//            cout << "main: Error: " << context->errstr << endl;
//            redisFree(context);
//        } else {
//            cout << "main: Can't allocate redis context" << endl;
//        }
//        return EXIT_FAILURE;
//    }
//
//
//    auto messages = Redis::readGroupMessages(context, "CC_1", "consumer", "cc_1", -1, 0);
//
//    // print size
//    cout << "main: messages.size(): " << messages.size() << endl;
//
//    // Print messages
//    for (const auto& message : messages) {
//        string messageId = get<0>(message);
//        cout << "Message: " <<messageId << endl;
//        for (const auto& field : get<1>(message)) {
//            cout << field.first << ": " << field.second << endl;
//        }
//
//        // Delete message
//        Redis::deleteMessage(context, "cc_1", get<0>(message));
//    }
//    redisFree(context);
//
//
//    return EXIT_SUCCESS;


    // Default values
    int height = 300;
    int width = 300;
    unsigned int num_drones = 9000;

    // signal handling


    // cc id
    unsigned int cc_id = 1;

    // Define long options
    static struct option long_options[] = {
            {"height", required_argument, nullptr, 'h'},
            {"width", required_argument, nullptr, 'w'},
            {nullptr, 0, nullptr, 0}
    };

    // Parse arguments
    int option_index = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "h:w:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                height = atoi(optarg);
                if (height <= 0) {
                    printHelp(argv[0]);
                    return EXIT_FAILURE;
                }
                break;
            case 'w':
                width = atoi(optarg);
                if (width <= 0) {
                    printHelp(argv[0]);
                    return EXIT_FAILURE;
                }
                break;
            default:
                printHelp(argv[0]);
                return EXIT_FAILURE;
        }
    }
    cout << "main: Height: " << height << " Width: " << width << endl;

    // Initialize an Area object
    Area area = Area(width, height);

    // Initialize a BasicStrategy object
    BasicStrategy strategy = BasicStrategy();

    // number of drones needed

    // Initialize a ControlCenter object
    ControlCenter controlCenter = ControlCenter(cc_id, num_drones, &strategy, area);

    // Initialize drones
    vector<Drone> drones;
    drones.reserve(num_drones);
    for (unsigned int i = 0; i < num_drones; i++) {
        drones.emplace_back(i, cc_id);
    }

    // Start drones
    cout << "main: Starting drones" << endl;
    vector<thread> drone_threads(drones.size());

    for (unsigned int i = 0; i < num_drones; i++) {
        drone_threads[i] = thread(&Drone::start, &drones[i]);
        // sleep for 1ms to
//        this_thread::sleep_for(chrono::milliseconds(7));
    }


    // Start control center
    cout << "Starting control center" << endl;
    thread cc_thread(&ControlCenter::start, &controlCenter);


    // Wait for all threads to finish

    cc_thread.join();

    cout << "All threads finished" << endl;

    return EXIT_SUCCESS;
}
