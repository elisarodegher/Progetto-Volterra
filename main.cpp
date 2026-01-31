#include <exception>
#include <iostream>
#include <limits>
#include <string>

#include "simulation.hpp"

int main() {
  try {
    std::string const menu{
        "- calculate evolution [c]\n"
        "- print evolution on file [se #SIGN-DIGITS]\n"
        "- draw trajectory on file [st]\n"
        "- print statistics on file [ss #SIGN-DIGITS]\n"
        "- quit execution [q]\n"};

    volterra::Simulation sim;
    std::cout << menu;

    int precision{};
    std::string cmd{};

    while (std::cin >> cmd) {
      if (cmd == "c") {
        sim.compute();

      } else if (cmd == "se" && std::cin >> precision) {  // SAVE EVOLUTION
        sim.save_evolution(precision, "beta.sim");

      } else if (cmd == "st") {  // SAVE TRAJECTORY
        sim.save_trajectory("beta.sim");

      } else if (cmd == "ss" && std::cin >> precision) {  // SAVE STATISTICS
        sim.save_statistics(precision, "beta.sim");

      } else if (cmd == "q") {
        return EXIT_SUCCESS;

      } else {
        std::cout << "Bad format, insert a new command\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }
    }

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
