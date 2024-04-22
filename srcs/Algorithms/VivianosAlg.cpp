#include "Algorithm.hpp"

struct Coordinates {
    int x;
    int y;

    Coordinates(int x_val, int y_val) : x(x_val), y(y_val) {}
};

Path::Path(std::vector<std::tuple<Direction, int>> path) : path_(path) {}

Path::Path() : path_(std::vector<std::tuple<Direction, int>>()) {}

std::vector<std::tuple<Direction, int>> Path::getPath() const {
    return path_;
}

void Path::addDirection(Direction dir, int steps) {
    path_.push_back(std::make_tuple(dir, steps));
}



class VivianosAlg : public Algorithm {
    public:
    // Implementazione del metodo computePath
    std::vector<int, Path> computePath(Field field, std::vector<<DroneData> drones) override {
        // TODO fare in modo che si salva il path una volta calcolato in modo che non deve più essere calcolato
        std::vector<int, Path> result;
        Field wrk_field = createWorkingField(field); 

        Coordinates init_pos = Coordinates(0,0) // init_pos = (0,0)
        Coordinates cc_pos = Coordinates(field.getCCPosition());

        int idx = 0;
        for (auto drone : drones) {
            while (true) {
                Coordinates cur_pos = init_pos;
                int autonomy -= ManhattanDistance(wrk_field.getCCPosition(), cur_pos);
                wrk_field.resetPointTimer(cur_pos.x, cur_pos.y);
                checkPoints(field, cur_pos);
                Coordinates drone_pos = calculateDronePosition(cur_pos);

                Path path = Path(std::vector<std::tuple<Direction, int>>());

                goToCurPos(); // TODO
                // TODO aggiungere a path il percorso per arrivare a cur_pos

                Direction next_dir;

                int steps;
                if (cur_pos.y == cc_pos.y) {
                    steps = (abs(cur_pos.x - cc_pos.x) + 1) * 10;
                    if (cur_pos.x <= cc_pos.x) { 
                        // UP
                        cur_pos.x = cur_pos.x;
                        cur_pos.y = cur_pos.y - steps;
                        autonomy -= steps;
                        path.addDirection(Direction.UP, steps);
                        next_dir = Direction.RIGHT; // andrà a destra dopo
                    } else { 
                        // DOWN
                        cur_pos.x = cur_pos.x;
                        cur_pos.y = cur_pos.y + steps;
                        autonomy -= steps;
                        path.addDirection(Direction.DOWN, steps);
                        next_dir = Direction.LEFT;
                    }
                }

                else if (cur_pos.x == cc_pos.x) {
                    steps = abs(cur_pos.y - cc_pos.y) + 1;
                    if (cur_pos.y <= cc_pos.y) { 
                        // RIGHT
                        cur_pos.x = cur_pos.x + steps;
                        cur_pos.y = cur_pos.y;
                        autonomy -= steps;
                        path.addDirection(Direction.RIGHT, steps*10);
                        next_dir = Direction.DOWN;
                    } else {
                        // LEFT
                        cur_pos.x = cur_pos.x - steps;
                        cur_pos.y = cur_pos.y;
                        autonomy -= steps;
                        path.addDirection(Direction.LEFT, steps*10);
                        next_dir = Direction.UP;
                    }
                }

                else {
                    steps = abs(cur_pos.x - cc_pos.x) + abs(cur_pos.y - cc_pos.y) + 1;
                    if (next_dir == UP) {
                        // UP
                        cur_pos.x = cur_pos.x;
                        cur_pos.y = cur_pos.y - steps;
                        autonomy -= steps;
                        path.addDirection(Direction.UP, steps*10);
                        next_dir = Direction.RIGHT;

                        if (autonomy - (steps + ManhattanDistance(cur_pos, cc_pos)) < 0) { // Va a destra in modo da ottimizzare il percorso del prossimo drone
                            autonomy -= ManhattanDistance(cur_pos, cc_pos);
                            path.addDirection(Direction.RIGHT, abs(cur_pos.x - cc_pos.x)*10);
                            // inizializza posizione per il prossimo drone
                            init_pos.x = cc_pos.x;
                            init_pos.y = cur_pos.y;
                            result.push_back(std::make_tuple(drone.id, path));
                            break;
                        }
                    }
                    elif (next_dir == DOWN) {
                        // DOWN
                        cur_pos.x = cur_pos.x;
                        cur_pos.y = cur_pos.y + steps;
                        autonomy -= steps;
                        path.addDirection(Direction.DOWN, steps*10);
                        next_dir = Direction.LEFT;

                        if (autonomy - (steps + ManhattanDistance(cur_pos, cc_pos)) < 0) { // Va a sinistra in modo da ottimizzare il percorso del prossimo drone
                            autonomy -= ManhattanDistance(cur_pos, cc_pos);
                            path.addDirection(Direction.LEFT, abs(cur_pos.x - cc_pos.x)*10);
                            // inizializza posizione per il prossimo drone
                            init_pos.x = cc_pos.x;
                            init_pos.y = cur_pos.y;
                            result.push_back(std::make_tuple(drone.id, path));
                            break;
                        }
                    }
                    elif (next_dir == RIGHT) {
                        // RIGHT
                        cur_pos.x = cur_pos.x + steps;
                        cur_pos.y = cur_pos.y;
                        autonomy -= steps;
                        path.addDirection(Direction.RIGHT, steps*10);
                        next_dir = Direction.DOWN;

                        if (autonomy - (steps + ManhattanDistance(cur_pos, cc_pos)) < 0) { // Va in basso in modo da ottimizzare il percorso del prossimo drone
                            autonomy -= ManhattanDistance(cur_pos, cc_pos);
                            path.addDirection(Direction.DOWN, abs(cur_pos.y - cc_pos.y)*10);
                            // inizializza posizione per il prossimo drone
                            init_pos.x = cur_pos.x;
                            init_pos.y = cc_pos.y;
                            result.push_back(std::make_tuple(drone.id, path));
                            break;
                        }
                    }
                    elif (next_dir == LEFT) {
                        // LEFT
                        cur_pos.x = cur_pos.x - steps;
                        cur_pos.y = cur_pos.y;
                        autonomy -= steps;
                        path.addDirection(Direction.LEFT, steps*10);
                        next_dir = Direction.UP;

                        if (autonomy - (steps + ManhattanDistance(cur_pos, cc_pos)) < 0) { // Va in alto in modo da ottimizzare il percorso del prossimo drone
                            autonomy -= ManhattanDistance(cur_pos, cc_pos);
                            path.addDirection(Direction.UP, abs(cur_pos.y - cc_pos.y)*10);
                            // inizializza posizione per il prossimo drone

                            init_pos.x = cur_pos.x;
                            init_pos.y = cc_pos.y;

                            result.push_back(std::make_tuple(drone.id, path));

                            break;
                        }
                    }
                }
            }
            idx++;
        }

        return result;
    }

    Field createWorkingField(Field field) {
        Field new_field = Field(field.getWidth() / 20, field.getHeight() / 20);
        return new_field;
    }

    int ManhattanDistance(Coordinates a, Coordinates b) { // TODO: funzione di utilità che servirà anche al drone per capire i punti checkati
        return abs(a.x - b.x) + abs(a.y - b.y);
    }

    void checkPoints(Field field, Coordinates cur_pos) { // TODO: non deve essere qua
        int x = cur_pos.x;
        int y = cur_pos.y;
        int startX = x == 0 ? 0 : x + 19; //0 se x = 0, altrimenti x + 19
        int endX = x * 20 + 19; // x * 20 + 19
        int startY = y == 0 ? 0 : y + 19; // 0 se y = 0, altrimenti y + 19
        int endY = y * 20 + 19; // y * 20 + 19

        for (int i = startX; i <= endX; ++i) {
            for (int j = startY; j <= endY; ++j) {
                field.resetPointTimer(i, j);
            }
        }
    }

    Coordinates calculateDronePosition(Coordinates cur_pos) {
        int startX = x == 0 ? 0 : x + 9; //0 se x = 0, altrimenti x + 19
        int endX = x * 10 + 9; // x * 20 + 19
        int startY = y == 0 ? 0 : y + 9; // 0 se y = 0, altrimenti y + 19
        int endY = y * 10 + 9; // y * 20 + 19
        return Coordinates((startX + endX) / 2, (startY + endY) / 2);
    }
}
