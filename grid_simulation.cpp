#include "grid_simulation.hpp"

namespace wator {

// COSTRUTTORE

GridSimulation::GridSimulation(GridParameters p, unsigned seed)
    : parameters_(p), rng_(seed) {
  if (p.width == 0 || p.height == 0 || p.iterations <= 0 ||
      p.fish_density <= 0 || p.sharks_density <= 0 ||
      (p.fish_density + p.sharks_density) > 1 || p.fish_density <= 0 ||
      p.sharks_density <= 0 || p.sharks_initial_energy <= 0 ||
      p.sharks_food_energy <= 0 || p.sharks_breed_energy <= 0 ||
      p.sharks_move_cost <= 0) {
    throw std::invalid_argument("Invalid input.");
  }

  grid_.resize(p.width * p.height);

  std::uniform_real_distribution<double> dist(0.0, 1.0);

  for (auto& cell : grid_) {
    double const r = dist(rng_);

    if (r < parameters_.fish_density) {
      cell.state = CellState::Prey;
    } else if (r < parameters_.fish_density + parameters_.sharks_density) {
      cell.state = CellState::Predator;
      cell.energy = parameters_.sharks_initial_energy;
    }
  }
  history_.push_back(get_population());
};

// ---------------METODI DELLA CLASSE------------------

// ---------------PRIVATI------------------------------

Population GridSimulation::get_population() const {
  Population population;

  for (Cell const& cell : grid_) {
    if (cell.state == CellState::Prey)
      ++population.fish;
    else if (cell.state == CellState::Predator)
      ++population.sharks;
  }

  return population;
}

std::size_t GridSimulation::index(std::size_t row, std::size_t col) const {
  return row * parameters_.width + col;
}

Cell& GridSimulation::current_cell(int row, int column) {
  return grid_[index(row, column)];
};

std::size_t GridSimulation::up(std::size_t row) const {
  return (row + parameters_.height - 1) % parameters_.height;
}

std::size_t GridSimulation::down(std::size_t row) const {
  return (row + 1) % parameters_.height;
}

std::size_t GridSimulation::left(std::size_t col) const {
  return (col + parameters_.width - 1) % parameters_.width;
}

std::size_t GridSimulation::right(std::size_t col) const {
  return (col + 1) % parameters_.width;
}

// PUBBLICI serve fare evolve e go

// EXTERNAL FUNCTIONS
bool operator==(Cell const& a, Cell const& b) {
  return a.state == b.state && a.energy == b.energy && a.age == b.age;
}

bool operator==(Population const& a, Population const& b) {
  return a.fish == b.fish && a.sharks == b.sharks;
}

}  // namespace wator
