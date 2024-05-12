#include <iostream>
#include <cstdlib> // For atoi()
#include <getopt.h> // For getopt_long
#include "src/control_center/ControlCenter.hpp"
#include "src/scanning_strategy/BasicStrategy.h"


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
    // Default values
    int height = 6000;
    int width = 6000;

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
    cout << "Height: " << height << " Width: " << width << endl;

    // Initialize an Area object
    Area area = Area(width, height);

    // Initialize a BasicStrategy object
    BasicStrategy strategy = BasicStrategy();

    // number of drones needed
    unsigned  int num_drones = 10;

    // cc id
    unsigned int cc_id = 1;
    // Initialize a ControlCenter object
    ControlCenter controlCenter = ControlCenter(cc_id, num_drones, &strategy, area);

    // Initialize drones
    vector<Drone> drones;
    drones.reserve(num_drones);
    for (unsigned int i = 0; i < num_drones; i++) {

        drones.emplace_back(i, cc_id);
    }

    // Start drones
    cout << "Starting drones" << endl;
    vector<thread> drone_threads;
    drone_threads.reserve(drones.size());
    for (Drone &drone : drones) {
        drone_threads.emplace_back(&Drone::start, &drone);
    }


    // Start control center
    cout << "Starting control center" << endl;
    thread cc_thread(&ControlCenter::start, &controlCenter);


    // Wait for all threads to finish
    for (thread &drone_thread : drone_threads) {
        drone_thread.join();
    }
    cc_thread.join();

    cout << "All threads finished" << endl;

    return EXIT_SUCCESS;
}
