#include "simulation.hpp"

namespace volterra {

// STRUCT
enum class CellState { Empty, Prey, Predator };

struct Cell {
  CellState state{CellState::Empty};
  int energy{0};  // solo per i predatori
  int age{0};     // per entrambi
};

struct Population {
  int fish;    // prey
  int sharks;  // predators
};

// CLASSE
class GridSimulation {
  Parameters parameters_;  // preso da simulation.hpp
  int width_;              // larghezza griglia
  int height_;             // altezza griglia
  int iterations_;

  std::vector<Cell> grid_;  // griglia "appiattita" in un vettore 1D
  std::vector<Population>
      history_;  // vettore che contiene il registro dei vari stati della
                 // popolazione, contiene le struct populations

 public:
  GridSimulation(Parameters p, int width, int height, double prey_density,
                 double predator_density, int iterations);

  void evolve();
  void go();

  // getters
  Parameters parameters() const { return parameters_; }
  int width() const { return width_; }
  int height() const { return height_; }
  int iterations() const { return iterations_; }
  std::vector<Population> const& history() const { return history_; }
  
};

}  // namespace volterra
