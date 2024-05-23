#include <iostream>
#include <cstdlib> // For atoi()
#include <getopt.h> // For getopt_long
#include "src/control_center/ControlCenter.hpp"
#include "src/scanning_strategy/BasicStrategy.h"
#include <atomic>
#include <csignal>
#include "src/config.h"
#include "src/server/Server.hpp"

using namespace std;

std::atomic<bool> stop(false);


void printHelp(char* name) {
    cout << "Usage: "<< name << endl;
    cout << "Options:" << endl;
    cout << "  --height <height>  Height of the area. Must be greater than 0." << endl;
    cout << "  --width <width>    Width of the area. Must be greater than 0." << endl;
    cout << "  --scan_range <scan_range>    Scan range of the drones. Must be greater than 0." << endl;
    cout << "  --autonomy <autonomy>    Autonomy of the drones. Must be greater than 0." << endl;
}

void signalHandler(int signum) {
    std::cout << "Ctrl+C premuto!" << std::endl;
    stop.store(true);
}

void stopControlCenter(unsigned int minutes) {
    // Sleep for the given number of minutes
    if (minutes <= 0) {
        return;
    }
    cout << "main: Sleeping for " << minutes << " minutes" << endl;

    std::this_thread::sleep_for(std::chrono::minutes(minutes));
    cout << "main: Time is up!" << endl;
    stop.store(true);
}

/**
 * This main takes the following arguments:
 * --height/-h: Height of the area
 * --width: Width of the area
 */
int main(int argc, char* argv[]) {
    // Stop the program when Ctrl+C is pressed
    signal(SIGINT, signalHandler);
    // Or stop the program afte x minutes
    int minutes = -1;



    std::cout << "Premi Ctrl+C per generare un segnale di interruzione." << std::endl;

    // Default values
    int height = -1;
    int width = -1;
    int scan_range = -1;
    int drone_autonomy = -1;
    int point_exp_time = -1;
    int num_drones = -1;

    // Define long options
    static struct option long_options[] = {
            {"height", required_argument, nullptr, 'h'},
            {"width", required_argument, nullptr, 'w'},
            {"scan_range", required_argument, nullptr, 'r'},
            {"autonomy", required_argument, nullptr, 'a'},
            {"point_exp_time", required_argument, nullptr, 'e'},
            {"num_drones", required_argument, nullptr, 'n'},
            {"time", required_argument, nullptr, 't'},
            {nullptr, 0, nullptr, 0}
    };

    // Parse arguments
    int option_index = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "h:w:r:a", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                height = stoi(optarg);
                if (height < 0) {
                    printHelp(argv[0]);
                    return EXIT_FAILURE;
                }
                break;

            case 'w':
                width = stoi(optarg);
                if (width < 0) {
                    printHelp(argv[0]);
                    return EXIT_FAILURE;
                }
                break;

            case 'r':
                scan_range = stoi(optarg);
                if (scan_range < 0) {
                    printHelp(argv[0]);
                    return EXIT_FAILURE;
                }
                break;

            case 'a':
                drone_autonomy = stoi(optarg);
                if (drone_autonomy < 0) {
                    printHelp(argv[0]);
                    return EXIT_FAILURE;
                }
                break;

            case 'e':
                point_exp_time = stoi(optarg);
                if (point_exp_time <= 0) {
                    printHelp(argv[0]);
                    return EXIT_FAILURE;
                }
                break;

            case 'n':
                num_drones = stoi(optarg);
                if (num_drones <= 0) {
                    printHelp(argv[0]);
                    return EXIT_FAILURE;
                }
                Config::NUMBER_OF_DRONES = num_drones;
                break;
            case 't':
                minutes = stoi(optarg);
                if (minutes <= 0) {
                    printHelp(argv[0]);
                    return EXIT_FAILURE;
                }
                break;

            default:
                printHelp(argv[0]);
                return EXIT_FAILURE;
        }
    }
    Config::TIME_TO_SCAN = (minutes>0) ? minutes : Config::TIME_TO_SCAN;
    Config::POINT_EXPIRATION_TIME = (point_exp_time>0) ? point_exp_time : Config::POINT_EXPIRATION_TIME;
    Config::NUMBER_OF_DRONES = (num_drones>0) ? num_drones : Config::NUMBER_OF_DRONES;
    Config::SCAN_RANGE = (scan_range>0) ? scan_range : Config::SCAN_RANGE;
    Config::DRONE_AUTONOMY = (drone_autonomy>0) ? drone_autonomy : Config::DRONE_AUTONOMY;
    Config::AREA_WIDTH = (width>0) ? width : Config::AREA_WIDTH;
    Config::AREA_HEIGHT = (height>0) ? height : Config::AREA_HEIGHT;



    Config::AREA_WIDTH = ceil(Config::AREA_WIDTH / (Config::SCAN_RANGE * 2));
    Config::AREA_HEIGHT = ceil(Config::AREA_HEIGHT / (Config::SCAN_RANGE * 2)); // every square represents the area scanned by a drone
    Config::DRONE_STEPS = static_cast<int>((Config::DRONE_AUTONOMY * Config::DRONE_SPEED * 1000 / 60) / 20); // represents the autonomy in steps

    cout << "main: Height: " << Config::AREA_HEIGHT << " Width: " << Config::AREA_WIDTH << endl;
    cout << "main: Drone steps: " << Config::DRONE_STEPS << endl;

    // Initialize an Area object
    Area area = Area(1, Config::AREA_WIDTH, Config::AREA_HEIGHT);

    // Initialize a BasicStrategy object
    BasicStrategy strategy = BasicStrategy();

    // cc id
    unsigned int cc_id = 1;

    // Initialize a ControlCenter object
    ControlCenter controlCenter = ControlCenter(cc_id, Config::NUMBER_OF_DRONES, &strategy, area);

    // Initialize drones
    vector<Drone> drones;
    drones.reserve(Config::NUMBER_OF_DRONES);
    for (unsigned int i = 0; i < Config::NUMBER_OF_DRONES; i++) {
        drones.emplace_back(i, cc_id);
    }

    // Start drones
    cout << "main: Starting drones" << endl;
    vector<thread> drone_threads(drones.size());

    for (unsigned int i = 0; i < Config::NUMBER_OF_DRONES; i++) {
        drone_threads[i] = thread(&Drone::start, &drones[i]);

    }


    // Start control center
    cout << "main:Starting control center" << endl;
    thread cc_thread(&ControlCenter::start, &controlCenter);

    // Start timer to stop the simulation
    thread stop_thread(stopControlCenter, Config::TIME_TO_SCAN);
    stop_thread.detach();

    while (!stop) {}
    cout << "main: Stopping threads: WAIT" << endl;

    controlCenter.stop();

    // Wait for all threads to finish
    cc_thread.join();
    cout << "main: Control center stopped" << endl;

    for (unsigned int i = 0; i < Config::NUMBER_OF_DRONES; i++) {
//        cout << "main: Stopping drone " << i << endl;
        if (drone_threads[i].joinable()) {
            drone_threads[i].join();
        }
    }
    cout << "main: All drones stopped" << endl;

    cout << "main: All threads finished" << endl;

    return EXIT_SUCCESS;
}
