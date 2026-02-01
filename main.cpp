
#include <string>
#include "simulation.hpp"

int main() {
  volterra::Simulation sim;

  sim.compute();
  sim.save_evolution("sim");
  sim.save_statistics("sim");

  return 0;
}
