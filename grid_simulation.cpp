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
    history_.push_back(get_population());
  }
};

}  // namespace wator
