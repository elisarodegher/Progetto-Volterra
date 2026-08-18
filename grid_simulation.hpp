#include <random>

#include "simulation.hpp"

namespace volterra {

// ----------------------------------------------STRUCT-------------------------------------------------------------------
enum class CellState { Empty, Prey, Predator };

struct Cell {
  CellState state{CellState::Empty};
  int energy{0};  // solo per i predatori
  int age{0};     // per entrambi
};

struct Population {
  size_t fish;    // prey = fish
  size_t sharks;  // predators = sharks
};

//------------------------------------------ CLASSE--------------------------------------------------------------------
class GridSimulation {
  Parameters parameters_;  // preso da simulation.hpp
  int width_;              // larghezza griglia
  int height_;             // altezza griglia
  int iterations_;

  std::vector<Cell> grid_;  // griglia "appiattita" in un vettore 1D
  std::vector<Population>
      history_;       // vettore che contiene il registro dei vari stati della
                      // popolazione, contiene le struct populations
  std::mt19937 rng_;  // generatore di numeri casuali



  // ----------------------------------------index calculations---------------------------------------------------------
  size_t index(int row, int col) const { return row * width_ + col; }

  Cell& current_cell(int row, int column) {
    return grid_[index(row, column)];
  };  // probabilmente da sistemare per conversioni implicite


  // ---------------------------------------changing cells with pacman effect ---------------------------------------------------------
  // reminder: higher rows have lower indexes!
  std::size_t up(std::size_t row) const {
    return (row + height_ - 1) % height_;
  }
  std::size_t down(std::size_t row) const { return (row + 1) % height_; }
  std::size_t left(std::size_t col) const {
    return (col + width_ - 1) % width_;
  }
  std::size_t right(std::size_t col) const { return (col + 1) % width_; }

 public:

 // ----------------------------------------------------costruttore-----------------------------------------------------------

  GridSimulation(Parameters p, int width, int height, double prey_density,
                 double predator_density, int iterations);


  // ------------------------------------------------getters----------------------------------------------------------------
  Parameters parameters() const { return parameters_; }
  int width() const { return width_; }
  int height() const { return height_; }
  int iterations() const { return iterations_; }
  std::vector<Population> const& history() const { return history_; }

  // --------------------------------------------evolution----------------------------------------------------------------
  void evolve();
  void go();
};


//---------------------------------------------------- operatori di confronto---------------------------------------------------
bool operator==(Cell const& a, Cell const& b) {
  return a.state == b.state && a.energy == b.energy && a.age == b.age;
}

bool operator==(Population const& a, Population const& b) {
  return a.fish == b.fish && a.sharks == b.sharks;
}

}  // namespace volterra
