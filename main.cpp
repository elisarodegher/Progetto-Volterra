#include <exception>
#include <iostream>
#include <string>

#include "simulation.hpp"

int main() {
  try {
    volterra::Simulation sim;

    sim.compute();               // calcola l'evoluzione
    sim.save_evolution("sim");   // salva evoluzione
    sim.save_trajectory("sim");  // salva traiettoria
    sim.save_statistics("sim");  // salva statistiche + grafici

    return EXIT_SUCCESS;

  } catch (std::invalid_argument const& e) {
    std::cerr << "Invalid argument: '" << e.what() << "'\n";
    return EXIT_FAILURE;

  } catch (std::exception const& e) {
    std::cerr << "Caught exception: '" << e.what() << "'\n";
    return EXIT_FAILURE;

  } catch (...) {
    std::cerr << "Caught unknown exception\n";
    return EXIT_FAILURE;
  }
}
