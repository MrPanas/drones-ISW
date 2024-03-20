#include "Algorithm.hpp"

// Implementazione di un algoritmo specifico
class AlgoritmoSpecifico : public Algorithm {
    public:
        void esegui() override {
            // Implementazione specifica dell'algoritmo
        }

        std::string nome() const override {
            return "Algoritmo Specifico";
        }
};