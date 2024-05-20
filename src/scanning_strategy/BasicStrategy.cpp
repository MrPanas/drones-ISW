#include "BasicStrategy.h"

using namespace std;

/**
 * @brief Construct a new Strategy for the movement of the drones in the area
 */
BasicStrategy::BasicStrategy() : ScanningStrategy() {
    cout << "BasicStrategy created" << endl;
}

/**
 * @brief Create the schedules for the drones to scan the area
 *
 * @param area The area to scan
 * @return vector<DroneSchedule> The schedules for the drones
 */
vector<DroneSchedule> BasicStrategy::createSchedules(Area area) {
    Coordinate cc_pos = {area.getWidth() / 2, area.getHeight() / 2};
    Coordinate current_pos = cc_pos;
    Coordinate end_pos = {area.getWidth()-1, area.getHeight()-1};
    Coordinate start_pos = {0, 0};

    vector<DroneSchedule> schedules;

    Direction current_direction = Direction::EAST;
    bool goEast = false;
    int path_id = 0;

    while (current_pos.y < area.getHeight()) {
        cout << "---------------------path_id: " << path_id << "---------------------" << endl;

        current_pos = cc_pos;
        int autonomy = Config::DRONE_STEPS;
        cout << "autonomy: " << autonomy << endl;
        Path path = Path();

        // Go to start position ______
        int steps = abs(current_pos.x - start_pos.x) + abs(current_pos.y - start_pos.y);
        tuple<Coordinate, bool> checkNext = goToPoint(autonomy, current_pos, start_pos, cc_pos, path, false);
        current_pos = get<0>(checkNext);
        bool comeBack = get<1>(checkNext);
        if (comeBack) {
            cerr << "Error: Not Enough Autonomy" << endl;
            return schedules;
        }
        autonomy -= steps;
        // ___________________________

        // Go to end position
        while (true) {
            Coordinate next_pos;
            if (current_direction == Direction::EAST) {
                // Go right while you can
                steps = area.getWidth() - current_pos.x - 1;
                next_pos = {current_pos.x + steps, current_pos.y};
                checkNext = goToPoint(autonomy, current_pos, next_pos, cc_pos, path, false);
                current_pos = get<0>(checkNext);
                comeBack = get<1>(checkNext);
                if (comeBack) {
                    start_pos = current_pos;
                    goToPoint(autonomy, current_pos, cc_pos, cc_pos, path, true);
                    break;
                }
                autonomy -= steps;
                current_direction = Direction::SOUTH;
                goEast = false;
            }

            else if (current_direction == Direction::WEST) {
                // Go left while you can
                steps = current_pos.x;
                next_pos = {current_pos.x - steps, current_pos.y};

                checkNext = goToPoint(autonomy, current_pos, next_pos, cc_pos, path, false);
                current_pos = get<0>(checkNext);
                comeBack = get<1>(checkNext);
                if (comeBack) {
                    start_pos = current_pos;
                    goToPoint(autonomy, current_pos, cc_pos, cc_pos, path, true);
                    break;
                }
                autonomy -= steps;
                current_direction = Direction::SOUTH;
                goEast = true;
            }

            else if (current_direction == Direction::SOUTH) {
                // Go down one step
                steps = 1;
                next_pos = {current_pos.x, current_pos.y + steps};

                checkNext = goToPoint(autonomy, current_pos, next_pos, cc_pos, path, false);
                current_pos = get<0>(checkNext);
                comeBack = get<1>(checkNext);
                if (comeBack) {
                    start_pos = current_pos;
                    goToPoint(autonomy, current_pos, cc_pos, cc_pos, path, true);
                    break;
                }
                autonomy -= steps;
                current_direction = goEast ? Direction::EAST : Direction::WEST;
            }

            // check if the drone has reached the end of the area
            if (current_direction == Direction::SOUTH) {
                if (current_pos.y == area.getHeight() - 1) {
                    goToPoint(autonomy, current_pos, cc_pos, cc_pos, path, true);
                    current_pos = {current_pos.x, current_pos.y + 1};
                    break;
                }
            }
        }
        cout << "path_id: " << path_id << ", path: " << path.toString() << endl;
        int time = static_cast<int>((Config::POINT_EXPIRATION_TIME - 10000)* TIME_ACCELERATION);
        schedules.emplace_back(path_id, path, chrono::milliseconds(time));
        path_id++;
    }
    return schedules;
}


/**
 * @brief Go to a specific point in the area
 * @param autonomy autonomy of the drone at the moment
 * @param current_pos current position of the drone
 * @param next_pos next position to reach
 * @param cc_pos position of the control center
 * @param path path to update
 * @param comeBack if the drone has to come back to the control center
 * @return
 */
tuple<Coordinate, bool> BasicStrategy::goToPoint(int autonomy, Coordinate current_pos, Coordinate next_pos, Coordinate cc_pos, Path& path, bool comeBack) {
    int steps = 0;
    while (current_pos.x < next_pos.x) {
         // go right until you reach the next position or you run out of autonomy
         if (manhattanDistance(current_pos, cc_pos) >= autonomy && !comeBack) {
             path.addDirection(Direction::EAST, steps);
             return {current_pos, true};
         }
         current_pos = {current_pos.x + 1, current_pos.y};
         steps++;
         autonomy--;
    }
    path.addDirection(Direction::EAST, steps);

    steps = 0;
    while (current_pos.x > next_pos.x) {
        // go left until you reach the next position or you run out of autonomy
        if (manhattanDistance(current_pos, cc_pos) >= autonomy && !comeBack) {
            path.addDirection(Direction::WEST, steps);
            return {current_pos, true};
        }
        current_pos = {current_pos.x - 1, current_pos.y};
        steps++;
        autonomy--;
    }
    path.addDirection(Direction::WEST, steps);

    steps = 0;
    while (current_pos.y < next_pos.y) {
        // go down until you reach the next position or you run out of autonomy
        if (manhattanDistance(current_pos, cc_pos) >= autonomy && !comeBack) {
            path.addDirection(Direction::SOUTH, steps);
            return {current_pos, true};
        }
        current_pos = {current_pos.x, current_pos.y + 1};
        steps++;
        autonomy--;
    }
    path.addDirection(Direction::SOUTH, steps);

    steps = 0;
    while (current_pos.y > next_pos.y) {
        // go up until you reach the next position or you run out of autonomy
        if (manhattanDistance(current_pos, cc_pos) >= autonomy && !comeBack) {
            path.addDirection(Direction::NORTH, steps);
            return {current_pos, true};
        }
        current_pos = {current_pos.x, current_pos.y - 1};
        steps++;
        autonomy--;
    }
    path.addDirection(Direction::NORTH, steps);

    return {current_pos, false};
}

/**
 * @brief Destroy the Basic Strategy:: Basic Strategy object
 */
BasicStrategy::~BasicStrategy() {
    cout << "BasicStrategy destroyed" << endl;
}



