#ifndef GRID_SIMULATION_HPP
#define GRID_SIMULATION_HPP

#include <iostream>
#include <random>
#include <vector>

namespace wator {

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

struct GridParameters {
  // Grid / simulation
  std::size_t width;
  std::size_t height;
  int iterations;

  // Initial population
  double fish_density;
  double sharks_density;

  // Prey
  int fish_breed_age;

  // Predator
  int sharks_initial_energy;
  int sharks_breed_energy;
  int sharks_food_energy;
  int sharks_move_cost;
};

//----------------CLASSE-----------------------
class GridSimulation {
  // attributi
  GridParameters parameters_;

  std::vector<Cell> grid_;  // griglia "appiattita" in un vettore 1D
  std::vector<Population> history_;
  std::mt19937 rng_;

  // ------------restituire la population x e y corrente)--------------
  Population get_population() const;

  // -----------index calculations-------------------------------------
  std::size_t index(std::size_t row, std::size_t col) const;
  Cell& current_cell(int row, int column);

  // ------------------------------changing cells with pacman effect
  //  reminder: higher rows have lower indexes!
  std::size_t up(std::size_t row) const;
  std::size_t down(std::size_t row) const;
  std::size_t left(std::size_t col) const;
  std::size_t right(std::size_t col) const;

 public:
  // ----------------------------------------------------costruttore-----------------------------------------------------------

  GridSimulation(GridParameters parameters, unsigned seed);

  // ------------------------------------------------getters----------------------------------------------------------------
  GridParameters parameters() const { return parameters_; }
  int width() const { return parameters_.width; }
  int height() const { return parameters_.height; }
  int iterations() const { return parameters_.iterations; }
  std::vector<Population> const& history() const { return history_; }

  // ------------------------------------evolution----------------------------------------------------------------
  void evolve();
  void go();
};

//----------------------- operatori di confronto--------------------------

bool operator==(Cell const& a, Cell const& b);
bool operator==(Population const& a, Population const& b);

}  // namespace wator

#endif