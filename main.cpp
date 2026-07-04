#include <iostream>
#include <string>

#include "simulation.hpp"

int main() {
  try {
    volterra::Parameters p;
    p.a = volterra::control("Enter prey birth rate: ");
    p.b = volterra::control("Enter prey death rate: ");
    p.c = volterra::control("Enter predator birth rate: ");
    p.d = volterra::control("Enter predator death rate: ");

    double x = volterra::control("Enter initial prey population: ");
    double y = volterra::control("Enter initial predator population: ");
    double dt = volterra::control("Enter time resolution dt: ");
    double it = volterra::control("Enter number of iterations: ");

    volterra::Simulation sim(p, x, y, dt, it);

    sim.go();
    sim.save_evolution();
    sim.save_plot();

  } catch (std::exception const& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}