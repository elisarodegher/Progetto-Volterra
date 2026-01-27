#include <fstream>
#include <sstream>
#include <string>

#include "project.lv.hpp"

int main() {
  try {
    struct Simulations {
      lv::Simulation sim;
      std::string name;
    };

    std::vector<Simulations> S{};

    if (system("ls simulations >> simulations_list.txt") != 0) {
      throw std::runtime_error{"Unable to access setting files"};
    }

    std::ifstream sim_list{"simulations_list.txt"};
    std::string file_name;
    std::string sim_name;

    while (lv::read(sim_list, file_name, '\n')) {
      try {
        std::getline(std::stringstream{file_name}, sim_name, '.');
        S.push_back(Simulations{lv::Simulation("simulations/" + file_name), "results/" + sim_name});
      } catch (const std::exception& e) {
        std::cerr << "Initialization through " << file_name << " failed:\n" << e.what() << '\n';
      }
    }

    if (std::remove("simulations_list.txt") != 0) {
      throw std::runtime_error{"Unable to delete file list"};
    }

    if (system("mkdir -p results") != 0) {
      throw std::runtime_error{"Unable to create results directory"};
    }

    for (auto& s : S) {
      try {
        s.sim.compute();
        s.sim.save_evolution(6, s.name);
        s.sim.save_trajectory(s.name);
        s.sim.save_statistics(6, s.name);
      } catch (const std::exception& e) {
        std::cerr << "Simulation " << s.name << " failed" << e.what() << '\n';
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