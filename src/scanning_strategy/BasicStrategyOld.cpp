#include "BasicStrategy.h"

using namespace std;


vector<DroneSchedule> BasicStrategy::createSchedules(Area area) {
    //int autonomy = 15000;

    Coordinate cc_pos = {area.getWidth() / 2, area.getHeight() / 2};
    Coordinate current_position = cc_pos;
    Coordinate end = {area.getWidth(), area.getHeight()};
    Coordinate start = {0, 0}; // TODO: uncomment
    // Coordinate start = {0, 1};
    // Coordinate end = {area.getWidth(), area.getHeight() - 1};

    vector<DroneSchedule> schedules;

    Direction direction = Direction::EAST;
    int path_id = 0;
    while (current_position.x != end.x && current_position.y <= end.y) {
        cout << "----------------path_id: " << path_id << "----------------" << endl;
        current_position = cc_pos;
        int autonomy = 750; // 15000/20 = 750
        Path path = Path();

        // cout << "Current Position: " << current_position.x << " " << current_position.y << endl;
        int steps = abs(start.x - cc_pos.x);
        // Go To Start ___________
        if (start.x < cc_pos.x) {
            // Go left
            path.addDirection(Direction::WEST, steps);
        } else {
            // Go right
            path.addDirection(Direction::EAST, steps);
        }
        autonomy -= steps;

        steps = abs(start.y - cc_pos.y);
        if (start.y < cc_pos.y) {
            // Go down
            path.addDirection(Direction::NORTH, steps);
        } else {
            // Go up
            path.addDirection(Direction::SOUTH, steps);
        }
        autonomy -= steps;

        current_position = start;


        // TODO mettere che se già non può tornare da errore
        // _______________________
        while(true) {
            Coordinate next_position = current_position;
            if (direction == Direction::EAST) {
                // va a destra
                steps = area.getWidth() - current_position.x - 1;
                next_position = {current_position.x + steps, current_position.y};
                if (manhattanDistance(next_position, cc_pos) > autonomy - steps) {
                    // va a destra finché può
                    cout << "escape 1" << endl;
                    tuple<Coordinate, int> tup = getAvailableCoord(autonomy, current_position, next_position, cc_pos);

                    current_position = get<0>(tup);
                    autonomy -= get<1>(tup);
                    path.addDirection(Direction::EAST, get<1>(tup));
                    cout << "Start: " << start.x << " " << start.y << " Current Position: " << current_position.x << " " << current_position.y << endl;
                    start = {current_position.x, current_position.y};
                    path.addPath(returnToCC(autonomy, current_position, cc_pos)); // Return To CC
                    cout << "autonomy prima di un break: " << autonomy << endl;
                    break;
                } else {
                    // va a destra al massimo e setta la prossima direzione a sinistra
                    current_position.x += steps;
                    autonomy -= steps;
                    // direction = Direction::WEST;
                    path.addDirection(Direction::EAST, steps);
                }
                // Go South
                // steps = min(11, area.getHeight() - current_position.y); // TODO: uncomment
                steps = min(1, area.getHeight() - current_position.y);
                next_position = {current_position.x, current_position.y + steps};

                tuple<Coordinate, int> tup = goSouth(autonomy, current_position, next_position, cc_pos);
                if (autonomy - steps != autonomy - get<1>(tup)) {
                    // va a sud finché può
                    cout << "escape 2" << endl;
                    current_position = get<0>(tup);
                    autonomy -= get<1>(tup);
                    path.addDirection(Direction::SOUTH, get<1>(tup));
                    cout << "Start: " << start.x << " " << start.y << " Current Position: " << current_position.x << " " << current_position.y << endl;
                    start = {current_position.x, current_position.y};
                    path.addPath(returnToCC(autonomy, current_position, cc_pos)); // return to CC
                    break;
                }
                current_position = get<0>(tup);
                autonomy -= get<1>(tup);
                path.addDirection(Direction::SOUTH, get<1>(tup));
                direction = Direction::WEST;


            } else if (direction == Direction::WEST) {
                // va a sinistra
                steps = current_position.x;
                next_position = {current_position.x - steps, current_position.y};
                if (manhattanDistance(next_position, cc_pos) > autonomy - steps) {
                    // va a sinistra finché può
                    cout << "Escape Ovest" << endl;
                    tuple<Coordinate, int> tup = getAvailableCoord(autonomy, current_position, next_position, cc_pos);
                    current_position = get<0>(tup);
                    autonomy -= get<1>(tup);
                    path.addDirection(Direction::WEST, get<1>(tup));
                    cout << "Start: " << start.x << " " << start.y << " Current Position: " << current_position.x << " " << current_position.y << endl;
                    start = {current_position.x, current_position.y};
                    path.addPath(returnToCC(autonomy, current_position, cc_pos)); // Return To CC
                    break;
                } else {
                    // va a sinistra al massimo
                    current_position.x -= steps;
                    autonomy -= steps;
                    direction = Direction::NORTH;
                    path.addDirection(Direction::WEST, steps);
                }
                // Go South
                // steps = min(11, area.getHeight() - current_position.y); // TODO: uncomment
                steps = min(1, area.getHeight() - current_position.y);
                next_position = {current_position.x, current_position.y + steps};
                tuple<Coordinate, int> tup = goSouth(autonomy, current_position, next_position, cc_pos);
                if (autonomy - steps != autonomy - get<1>(tup)) {
                    // va a sud finché può
                    cout << "escape SUD/W" << endl;
                    current_position = get<0>(tup);
                    autonomy -= get<1>(tup);
                    path.addDirection(Direction::SOUTH, get<1>(tup));
                    cout << "Start: " << start.x << " " << start.y << " Current Position: " << current_position.x << " " << current_position.y << endl;
                    start = {current_position.x, current_position.y};
                    path.addPath(returnToCC(autonomy, current_position, cc_pos)); // return to CC
                    break;
                }
                // va a sud al massimo e setta la prossima direzione a destra
                current_position = get<0>(tup);
                autonomy -= get<1>(tup);
                path.addDirection(Direction::SOUTH, get<1>(tup));
                direction = Direction::EAST;
            }
        }

        // cout << "Start: " << start.x << " " << start.y << " Current Position: " << current_position.x << " " << current_position.y << endl;
        cout << "pathId: " << path_id << " Path: " << path.toString() << endl;
        schedules.emplace_back(path_id, path, chrono::milliseconds(290000)); //TODO: mettere 290000
        path_id++;
    }
    return schedules;
}

Path BasicStrategy::returnToCC(int autonomy, Coordinate current_position, Coordinate cc_pos) {
    Path path = Path();
    if (current_position.x < cc_pos.x) {
        // Go right
        path.addDirection(Direction::EAST, abs(current_position.x - cc_pos.x));
    } else {
        // Go left
        path.addDirection(Direction::WEST, abs(current_position.x - cc_pos.x));
    }

    if (current_position.y < cc_pos.y) {
        // Go North
        path.addDirection(Direction::SOUTH, abs(current_position.y - cc_pos.y));
    } else {
        // Go South
        path.addDirection(Direction::NORTH, abs(current_position.y - cc_pos.y));
    }
    return path;
}


tuple<Coordinate, int> BasicStrategy::getAvailableCoord(int autonomy, Coordinate current_position, Coordinate next_position, Coordinate cc_pos) {
    int steps = 0;
    while (current_position.x != next_position.x) {
        Coordinate temp;
        if (current_position.x < next_position.x) {
            temp = {current_position.x + 1, current_position.y};
            if (manhattanDistance(temp, cc_pos) > autonomy - 1) {
                return make_tuple(current_position, steps);
            } else {
                steps++;
                autonomy--;
                current_position = temp; // Move to the next position
            }

        } else {
            temp = {current_position.x - 1, current_position.y};
            if (manhattanDistance(temp, cc_pos) > autonomy - 1) {
                return make_tuple(current_position, steps);
            } else {
                steps++;
                autonomy--;
                current_position = temp; // Move to the next position
            }
        }
    }

    return make_tuple(current_position, steps);
}

tuple<Coordinate, int> BasicStrategy::goSouth(int autonomy, Coordinate current_position, Coordinate next_position, Coordinate cc_pos) {
    int steps = 0;
    while (current_position.y != next_position.y) {
        Coordinate temp{};
        if (current_position.y < next_position.y) {
            temp = {current_position.x, current_position.y + 1};
            if (manhattanDistance(temp, cc_pos) > autonomy) {
                return make_tuple(current_position, steps);
            } else {
                steps++;
                current_position = temp; // Move to the next position
            }

        } else {
            temp = {current_position.x, current_position.y - 1};
            if (manhattanDistance(temp, cc_pos) > autonomy) {
                return make_tuple(current_position, steps);
            } else {
                steps++;
                current_position = temp; // Move to the next position
            }
        }
    }
    return make_tuple(current_position, steps);
}


int BasicStrategy::manhattanDistance(Coordinate a, Coordinate b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

