#include "simulation.hpp"

namespace volterra {

enum class CellState { Empty, Prey, Predator };

struct Cell {
  CellState state{CellState::Empty};
  int energy{0};  // solo per i predatori
  int age{0};     // per entrambi
};

struct Population {
  int x;  // prey
  int y;  // predators
};

class GridSimulation {
  Parameters parameters_;  // preso da simulation.hpp
  int width_;              // larghezza griglia
  int height_;             // altezza griglia
  int iterations_;

  std::vector<Cell> grid_;  // griglia "appiattita" in un vettore 1D
  std::vector<Population> history_;

  
  public:
  GridSimulation(Parameters p, int width, int height, double prey_density,
                 double predator_density, int iterations);

  void evolve();
  void go();
};


}  
