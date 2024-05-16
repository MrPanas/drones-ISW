#include "BasicStrategy.h"

using namespace std;

vector<DroneSchedule> BasicStrategy::createSchedules(Area area) {
    Coordinate cc_pos = {area.getWidth() / 2, area.getHeight() / 2};
    Coordinate current_pos = cc_pos;
    Coordinate end_pos = {area.getWidth()-1, area.getHeight()-1};
    Coordinate start_pos = {0, 0};

    vector<DroneSchedule> schedules;

    Direction current_direction = Direction::EAST;
    bool goEast = false;
    int path_id = 0;

    while (current_pos != end_pos) {
        cout << "---------------------path_id: " << path_id << "---------------------" << endl;
        current_pos = cc_pos;
        int autonomy = DRONE_AUTONOMY; // 15000/20 = 750
        Path path = Path();

        // Go to start position ______
        tuple<Coordinate, bool> checkNext = goToPoint(autonomy, current_pos, start_pos, cc_pos, path, false);
        current_pos = get<0>(checkNext);
        bool comeBack = get<1>(checkNext);
        if (comeBack) {
            // TODO: cambiare e mettere che da errore se va qua
            goToPoint(autonomy, current_pos, cc_pos, cc_pos, path, true);
            break;
        }
        int steps = abs(current_pos.x - start_pos.x) + abs(current_pos.y - start_pos.y);
        autonomy -= steps;
        // ___________________________

        while (true) {
            // check if the drone reach the end of the area
            // TODO creare una funzione apposita
            switch(current_direction) {
                case Direction::EAST:
                    if (current_pos.x == area.getWidth() - 1) {
                        break; // TODO: questo break chiude il while giusto?
                    }
                    break;
                case Direction::SOUTH:
                    if (current_pos.y == area.getHeight() - 1) {
                        break;
                    }
                    break;
                case Direction::WEST:
                    if (current_pos.x == 0) {
                        break;
                    }
                    break;
                case Direction::NORTH:
                    if (current_pos.y == 0) {
                        break;
                    }
                    break;
            }


            Coordinate next_pos = current_pos;
            if (current_direction == Direction::EAST) {
                // va a destra
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
        }
        cout << "path_id: " << path_id << ", path: " << path.toString() << endl;
        schedules.emplace_back(path_id, path, chrono::milliseconds(290000));
        path_id++;
    }
    return schedules;
}

tuple<Coordinate, bool> BasicStrategy::goToPoint(int autonomy, Coordinate current_pos, Coordinate next_pos, Coordinate cc_pos, Path path, bool comeBack) {
    // TODO prova ad andare a next_position se non riesce va dove può e torna al CC e aggiorna anche il path
    int steps = 0;
    while (current_pos.x < next_pos.x) {
         // go right
         if (manhattanDistance(current_pos, cc_pos) >= autonomy && !comeBack) {
             path.addDirection(Direction::EAST, steps);
             return {current_pos, true};
         }
         current_pos = {current_pos.x + 1, current_pos.y};
         steps++;
    }
    path.addDirection(Direction::EAST, steps);

    steps = 0;
    while (current_pos.x > next_pos.x) {
        // go left
        if (manhattanDistance(current_pos, cc_pos) >= autonomy && !comeBack) {
            path.addDirection(Direction::WEST, steps);
            return {current_pos, true};
        }
        current_pos = {current_pos.x - 1, current_pos.y};
        steps++;
    }
    path.addDirection(Direction::WEST, steps);

    steps = 0;
    while (current_pos.y < next_pos.y) {
        // go down
        if (manhattanDistance(current_pos, cc_pos) >= autonomy && !comeBack) {
            path.addDirection(Direction::SOUTH, steps);
            return {current_pos, true};
        }
        current_pos = {current_pos.x, current_pos.y + 1};
        steps++;
    }
    path.addDirection(Direction::SOUTH, steps);

    steps = 0;
    while (current_pos.y > next_pos.y) {
        // go up
        if (manhattanDistance(current_pos, cc_pos) >= autonomy && !comeBack) {
            path.addDirection(Direction::NORTH, steps);
            return {current_pos, true};
        }
        current_pos = {current_pos.x, current_pos.y - 1};
        steps++;
    }
    path.addDirection(Direction::NORTH, steps);

    return {current_pos, false};
}

int BasicStrategy::manhattanDistance(Coordinate a, Coordinate b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}



