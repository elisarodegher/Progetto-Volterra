#ifndef GRID_SIMULATION_HPP
#define GRID_SIMULATION_HPP

#include <array>
#include <iostream>
#include <random>
#include <vector>

namespace wator {

enum class CellState { Empty, Prey, Predator };

//--------------------------- STRUCT ---------------------------
struct Cell {
  CellState state{CellState::Empty};
  int energy{0};
  int age{0};
};

struct Population {
  size_t fish{0};
  size_t sharks{0};
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

// --------------------------- CLASS ---------------------------

class GridSimulation {
  // attributes
  GridParameters parameters_;
  std::vector<Cell> grid_;
  std::vector<Population> history_;
  std::mt19937 rng_;

  //-----------------------PRIVATE FUNCTIONS-------------------------

  Population get_population() const;

  std::size_t index(std::size_t row, std::size_t col) const;
  Cell& current_cell(size_t row, size_t column);

  // changing cells with pacman effect

  std::size_t up(std::size_t row) const;
  std::size_t down(std::size_t row) const;
  std::size_t left(std::size_t col) const;
  std::size_t right(std::size_t col) const;

  std::vector<std::size_t> neighbours(std::size_t row, std::size_t col,
                                      CellState state) const;
  void reproduce(std::size_t index, std::vector<std::size_t> const& free_neighb,
                 std::size_t reproduction_treshold);

 public:
  //-------------------------CONSTRUCTOR------------------------------

  GridSimulation(GridParameters parameters, unsigned seed);

  //-----------------------PUBLIC FUNCTIONS-------------------------

  GridParameters parameters() const { return parameters_; }
  size_t width() const { return parameters_.width; }
  size_t height() const { return parameters_.height; }
  size_t iterations() const { return parameters_.iterations; }
  std::vector<Population> const& history() const { return history_; }

  // evolution
  void evolve();
  void go();

  // output
  void save_grid_evolution();
  void save_grid_plot();
};

//--------------------EXTERNAL FUNCTIONS------------------------

// operators
bool operator==(Cell const& a, Cell const& b);
bool operator==(Population const& a, Population const& b);
bool operator==(GridParameters const& a, GridParameters const& b);

}  // namespace wator

#endif