
#include <string>

#include "simulation.hpp"

int main() {
  volterra::Simulation sim;
  sim.go();
  sim.save_evolution();
  sim.save_plot();

  return 0;
}