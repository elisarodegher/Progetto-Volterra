#include "simulation.hpp"

int main() {
  try {
    lv::Simulation s(lv::Parameters{2000., 4., 8., 1000.}, 200., 100., 0.00001, 100000.);

    s.compute();

    if (system("mkdir -p results") != 0) {
      throw std::runtime_error{"Unable to create results directory"};
    }

    s.save_evolution(6, "results/alpha.sim");
    s.save_trajectory("results/alpha.sim");
    s.save_statistics(6, "results/alpha.sim");

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