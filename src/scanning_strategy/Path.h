#ifndef DRONE8_PATH_H
#define DRONE8_PATH_H

#include <vector>
#include <tuple>
#include <string>

using namespace std;

/**
 * Enum for the directions
 */
enum Direction {
    NORTH,
    SOUTH,
    WEST,
    EAST
};

/**
 * Convert a direction to a string
 * @param direction
 * @return direction as string
 */
inline string toString(Direction direction) {
    switch (direction) {
        case NORTH:
            return "N";
        case SOUTH:
            return "S";
        case WEST:
            return "W";
        case EAST:
            return "E";
        default:
            return "U";
    }
}

/**
 * Path class
 */
class Path {
    using DirectionList = vector<tuple<Direction, int>>;
public:
    Path(DirectionList path);
    Path();

    DirectionList getPath() const;

    void addDirection(Direction dir, int steps);

    string toString() const;

private:
    DirectionList path_;
};


#endif //DRONE8_PATH_H
