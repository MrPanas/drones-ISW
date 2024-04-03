#include "Algorithm.hpp"
#include "Direction.hpp" // Assumendo che Direction.hpp definisca l'enum Direction

class MyAlgorithm : public Algorithm {
public:
    // Implementazione del metodo computePath
    std::vector<std::tuple<Direction, int>> computePath(int droneId) override {
        // Implementazione della logica per calcolare il percorso
        // In questo esempio, restituiamo solo una lista vuota di tuple
        return std::vector<std::tuple<Direction, int>>();
    }
};