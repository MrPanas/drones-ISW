#include "Algorithm.hpp"
#include <iostream>

class MyAlgorithm : public Algorithm {
public:
    // Implementazione del metodo computePath
    std::vector<std::tuple<Direction, int>> computePath(int droneId) override {
        // Implementazione della logica per calcolare il percorso
        // In questo esempio, restituiamo solo una lista vuota di tuple
        std::cout << "Drone " << droneId << " is using MyAlgorithm" << std::endl;
        return std::vector<std::tuple<Direction, int>>();
    }
};