#include <iostream>
#include <string>

#include "grid_simulation.hpp"
#include "simulation.hpp"

namespace wator {

template <typename T>
T control_as(std::string const& message) {
  double const v = volterra::control(message);
  if (std::floor(v) != v) throw std::invalid_argument("Invalid input.");
  return static_cast<T>(v);
}

void run_continuous_simulation() {
  volterra::Parameters p;
  p.a = volterra::control("Enter prey birth rate: ");
  p.b = volterra::control("Enter prey death rate: ");
  p.c = volterra::control("Enter predator birth rate: ");
  p.d = volterra::control("Enter predator death rate: ");

  double const x = volterra::control("Enter initial prey population: ");
  double const y = volterra::control("Enter initial predator population: ");
  double const dt = volterra::control("Enter time resolution dt: ");
  double const it = volterra::control("Enter number of iterations: ");

  volterra::Simulation sim(p, x, y, dt, it);

  sim.go();
  sim.save_evolution();
  sim.save_plot();
}

void run_grid_simulation() {
  wator::GridParameters p;
  p.width = control_as<std::size_t>("Enter grid width: ");
  p.height = control_as<std::size_t>("Enter grid height: ");
  p.iterations = control_as<std::size_t>("Enter number of iterations: ");

  p.fish_density = volterra::control("Enter initial fish density (0-1): ");
  p.sharks_density = volterra::control("Enter initial sharks density (0-1): ");

  p.fish_breed_age = control_as<int>("Enter fish breeding age: ");

  p.sharks_initial_energy = control_as<int>("Enter sharks initial energy: ");
  p.sharks_breed_energy =
      control_as<int>("Enter sharks reproduction energy threshold: ");
  p.sharks_food_energy =
      control_as<int>("Enter energy gained per fish eaten: ");
  p.sharks_move_cost = control_as<int>("Enter energy lost per step: ");

  unsigned const seed = control_as<unsigned>("Enter random seed: ");

  wator::GridSimulation sim(p, seed);
  sim.go();

  // TODO: sim.save_evolution(); sim.save_plot(); -- ancora da implementare

  auto const& last = sim.history().back();
  std::cout << "\nSimulazione conclusa dopo " << sim.history().size()
            << " passi.\nPopolazione finale: " << last.fish << " prede, "
            << last.sharks << " predatori.\n";
}

}  // namespace wator

int main() {
  try {
   } catch (std::exception const& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}