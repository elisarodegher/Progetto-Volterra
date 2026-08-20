#ifndef GRID_SIMULATION_HPP
#define GRID_SIMULATION_HPP

#include <array>
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
  size_t fish{0};    // prey = fish
  size_t sharks{0};  // predators = sharks
};

struct GridParameters {
  // Grid / simulation
  std::size_t width;
  std::size_t height;
  std::size_t iterations;

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
  Cell& current_cell(size_t row, size_t column);

  // ------------------------------changing cells with pacman effect
  //  reminder: higher rows have lower indexes!
  std::size_t up(std::size_t row) const;
  std::size_t down(std::size_t row) const;
  std::size_t left(std::size_t col) const;
  std::size_t right(std::size_t col) const;

  void reproduce(std::size_t index, std::vector<std::size_t> const& free_neighb,
                 std::size_t reproduction_treshold);

  std::vector<std::size_t> empty_neighbours(std::size_t row,
                                            std::size_t col) const;

 public:
  // ----------------------------------------------------costruttore-----------------------------------------------------------

  GridSimulation(GridParameters parameters, unsigned seed);

  // ------------------------------------------------getters----------------------------------------------------------------
  GridParameters parameters() const { return parameters_; }
  size_t width() const { return parameters_.width; }
  size_t height() const { return parameters_.height; }
  size_t iterations() const { return parameters_.iterations; }
  std::vector<Population> const& history() const { return history_; }

  // ------------------------------------evolution----------------------------------------------------------------
  void evolve();
  void go();
};

//----------------------- operatori di confronto--------------------------

bool operator==(Cell const& a, Cell const& b);
bool operator==(Population const& a, Population const& b);
bool operator==(GridParameters const& a, GridParameters const& b);

}  // namespace wator

#endif