#include "Algorithm.hpp"
#include <iostream>

class MyAlgorithm : public Algorithm {
public:
    // Implementazione del metodo computePath
    std::vector<std::tuple<int, Path>> computePath(Field field, std::vector<DroneData> drones) override {
        // Implementazione della logica per calcolare il percorso
        // In questo esempio, restituiamo solo una lista vuota di tuple
        std::cout << "Drone is using MyAlgorithm" << std::endl;
        std::cout << "Field height: " << field.getHeight() << std::endl;
        std::vector<DroneData> dronesss = drones;

        return std::vector<std::tuple<int, Path>>();
    }
};