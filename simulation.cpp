#include "project.lv.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <numeric>

namespace lv {
Simulation Simulation::set_simulation() {
  try {
    static std::string comm_list{
        "LIST OF VALID COMMANDS\n"
        "parameters:\n"
        "- set prey birth rate [a VAL]\n"
        "- set prey death rate [b VAL]\n"
        "- set predator birth rate [c VAL]\n"
        "- set predator death rate [d VAL]\n"
        "populations\n"
        "- set prey [prey VAL]\n"
        "- set predator [predator VAL]\n"
        "evolution\n"
        "- set discretization fineness [t VAL]\n"
        "- set number of iterations [n VAL]\n"
        "- quit simulation setting[q]\n"
        "- help [h]\n"};

    Parameters p{};
    double prey{};
    double pred{};
    double dt{};
    double n{};

    std::string opt;

    std::vector<std::string> control{"a", "b", "c", "d", "prey", "predator", "t", "n"};

    std::cout << comm_list;

    while (control != std::vector<std::string>{"1", "1", "1", "1", "1", "1", "1", "1"}) {
      std::cin >> opt;

      if (opt == "a" && std::cin >> p.A) {  // PREY BIRTH RATE SETTING
        control[0] = "1";

      } else if (opt == "b" && std::cin >> p.B) {  // PREY DEATH RATE SETTING
        control[1] = "1";

      } else if (opt == "c" && std::cin >> p.C) {  // PREDATOR BIRTH RATE SETTING
        control[2] = "1";

      } else if (opt == "d" && std::cin >> p.D) {  // PREDATOR DEATH RATE SETTING
        control[3] = "1";

      } else if (opt == "prey" && std::cin >> prey) {  // PREY INITIAL POPULATION SETTING
        control[4] = "1";

      } else if (opt == "predator" && std::cin >> pred) {  // PREDATOR INITIAL POPULATION SETTING
        control[5] = "1";

      } else if (opt == "t" && std::cin >> dt) {  // DISCRETIZATION SETTING
        control[6] = "1";

      } else if (opt == "n" && std::cin >> n) {  // #ITERATIONS SETTING
        control[7] = "1";

      } else if (opt == "q") {  // WARNS INITIALIZATION INCOMPLETENESS
        std::cout << "WARNING - Initialization incomplete.\nMissing:\n";

        for (auto const& ins : control) {
          if (ins != "1") {
            std::cout << ins << '\n';
          }
        }

        std::cout << "Object construction will fail.\n"
                  << "Confirm quitting? [ y / * ]\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin >> opt;

        if (opt == "y") {
          throw std::invalid_argument{"Unspecified initial conditions."};
        } else {
          std::cout << "Continue setting\n";
        }
      } else if (opt == "h") {
        std::cout << comm_list;
      } else {
        std::cerr << "Bad format, insert a new command\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }
    }

    return Simulation{is_good(p), is_good(prey), is_good(pred), dt_good(dt), it_good(n)};

  } catch (const std::invalid_argument& e) {
    throw std::invalid_argument{e.what()};
  } catch (const std::runtime_error& e) {
    throw std::runtime_error{e.what()};
  } catch (...) {
    throw std::runtime_error{"Caught unknown exception during initialization"};
  }
}

Simulation Simulation::set_simulation(std::string const& file_name) {
  try {
    Parameters p{};
    double prey{};
    double pred{};
    double dt{};
    double n{};

    std::ifstream file(file_name);

    if (!file.is_open()) {
      throw std::runtime_error{"Impossible to open initialization file"};
    }

    std::string line;
    std::string opt;
    std::vector<std::string> control{"a", "b", "c", "d", "prey", "predator", "t", "n"};

    while (read(file, line, '\n')) {
      std::string tmp;
      std::stringstream iss(line);

      iss >> opt;

      if (opt == "a" && iss >> tmp) {  // PREY BIRTH RATE SETTING
        p.A = std::stod(tmp);
        control[0] = "1";

      } else if (opt == "b" && iss >> tmp) {  // PREY DEATH RATE SETTING
        p.B = std::stod(tmp);
        control[1] = "1";

      } else if (opt == "c" && iss >> tmp) {  // PREDATOR BIRTH RATE SETTING
        p.C = std::stod(tmp);
        control[2] = "1";

      } else if (opt == "d" && iss >> tmp) {  // PREDATOR DEATH RATE SETTING
        p.D = std::stod(tmp);
        control[3] = "1";

      } else if (opt == "prey" && iss >> tmp) {  // PREY INITIAL POPULATION SETTING
        prey = std::stod(tmp);
        control[4] = "1";

      } else if (opt == "predator" && iss >> tmp) {  // PREDATOR INITIAL POPULATION SETTING
        pred = std::stod(tmp);
        control[5] = "1";

      } else if (opt == "t" && iss >> tmp) {  // DISCRETIZATION SETTING
        dt = std::stod(tmp);
        control[6] = "1";

      } else if (opt == "n" && iss >> tmp) {  // #ITERATIONS SETTING
        n = std::stod(tmp);
        control[7] = "1";
      }
    }
    // CHECKS INITIALIZATION COMPLETENESS
    if (control != std::vector<std::string>{"1", "1", "1", "1", "1", "1", "1", "1"}) {
      throw std::runtime_error{"Variable list is incomplete, unable to create object"};
    }
    file.close();
    if (file.is_open()) {
      throw std::runtime_error{"Unable to exit initialization file"};
    }

    return Simulation{is_good(p), is_good(prey), is_good(pred), dt_good(dt), it_good(n)};

  } catch (const std::invalid_argument& e) {
    throw std::invalid_argument{e.what()};
  } catch (const std::runtime_error& e) {
    throw std::runtime_error{e.what()};
  } catch (...) {
    throw std::runtime_error{"Caught unknown exception during initialization"};
  }
}

Parameters const& Simulation::is_good(Parameters const& p) {
  if (!((p.A > 0) && (p.B > 0) && (p.C > 0) && (p.D > 0))) {
    throw std::invalid_argument{"Parameters must be strictly greater than zero"};
  }
  return p;
}

double Simulation::is_good(double const val) {
  if (!(val > 0)) {
    throw std::invalid_argument{
        "For the sake of simplicity, we are already accepting\n"
        "to have a non integer number of individuals,\na negative"
        " population is way beyond acceptability"};
  }
  return val;
}

double Simulation::dt_good(double const t) {
  if (!(t > 0)) {
    throw std::invalid_argument{"A time intervall should really be positive"};
  } else if (!(t < 0.001)) {
    std::clog << "Quality-Warning: A large time increment produces less stable simulations\n";
  }
  return t;
}

std::size_t Simulation::it_good(double const val) {
  if (!(val > 0)) {
    throw std::invalid_argument{"Iterations should really be a natural number"};
  }
  return static_cast<std::size_t>(val);
}

void Simulation::evolve() {
  auto x = evolution_.back().prey;
  auto y = evolution_.back().predator;

  assert((0 < x && 0 < y));

  auto x_rel = state_.prey;
  auto y_rel = state_.predator;

  state_ = State{x_rel + par_.A * (1 - y_rel) * x_rel * dt_, y_rel + par_.D * (x_rel - 1) * y_rel * dt_,
                 (par_.C * x) + (par_.B * y) - (par_.D * std::log(x)) - (par_.A * std::log(y))};

  if (!(state_.prey > 0) || !(state_.predator > 0)) {
    state_ = State{x_rel, y_rel, evolution_.back().H};
    throw std::runtime_error{"WWF joined the simulation.\n"};
  } else {
    evolution_.push_back(State{state_.prey * par_.D / par_.C, state_.predator * par_.A / par_.B, state_.H});
  }
}

void Simulation::compute() {
  for (; evolution_.size() < iterations_;) {
    try {
      this->evolve();
    } catch (const std::runtime_error& e) {
      std::cerr << e.what() << '\n';
      break;
    }
  }
}

void Simulation::statistics() {
  struct Sums {
    double sum_prey;
    double sum_pred;
    double sum_prey2;
    double sum_pred2;
  };

  auto begin = evolution_.begin();
  auto end = evolution_.end();

  Sums sum = std::accumulate(begin, end, Sums{0., 0., 0., 0.}, [](Sums s, State p) {
    return Sums{(s.sum_prey + p.prey), (s.sum_pred + p.predator), (s.sum_prey2 + p.prey * p.prey),
                (s.sum_pred2 + p.predator * p.predator)};
  });

  double N{static_cast<double>(evolution_.size())};
  double mean{sum.sum_prey / N};

  prey_stat_ =
      Statistics{mean, std::sqrt(sum.sum_prey2 / N - std::pow(mean, 2.)),
                 (*std::max_element(begin, end, comp_prey)).prey, (*std::min_element(begin, end, comp_prey)).prey};

  mean = sum.sum_pred / N;

  pred_stat_ = Statistics{mean, std::sqrt(sum.sum_pred2 / N - std::pow(mean, 2.)),
                          (*std::max_element(begin, end, comp_pred)).predator,
                          (*std::min_element(begin, end, comp_pred)).predator};
}

void Simulation::show_evolution(int p) const {
  if (evolution_.size() < 2) {
    std::cerr << "No evolution to display.";
  } else {
    std::cout << "EVOLUTION OF THE ECOSYSTEM\n";
    std::cout << "--------------------------\n";
    for (auto const& s : evolution_) {
      std::cout << "prey: " << std::setprecision(p) << s.prey << "\t\t"
                << "H: " << std::setprecision(p) << s.H << '\n'
                << "predator: " << std::setprecision(p) << s.predator << '\n'
                << "--------------------------\n";
    }
  }
}

void Simulation::save_evolution(int p, std::string const& name) const {
  if (evolution_.size() < 2) {
    std::cerr << "No evolution to display.";
  } else {
    std::ofstream outfile{name + ".evolution.csv"};
    if (!outfile) {
      throw std::runtime_error{"Impossible to open output file"};
    }
    outfile << "prey\tpredator\tH\n";
    for (auto const& s : evolution_) {
      outfile << std::setprecision(p) << s.prey << '\t' << std::setprecision(p) << s.predator << '\t'
              << std::setprecision(p) << s.H << '\n';
    }
    std::cout << "Evolution saved in " << name << ".evolution.csv\n";
  }
}

void Simulation::show_trajectory() const {
  if (evolution_.size() < 2) {
    std::cerr << "No evolution to display.";
  } else {
    std::ofstream data_file{".tmp.csv"};  // PREPARE DATA TO PLOT

    if (!data_file) {
      throw std::runtime_error{"Impossible to open temporary data file"};
    }

    for (auto const& s : evolution_) {
      data_file << s.prey << '\t' << s.predator << '\n';
    }

    data_file.flush();

    const double xrange{((*std::max_element(evolution_.begin(), evolution_.end(), comp_prey)).prey * 1.05)};
    const double yrange{((*std::max_element(evolution_.begin(), evolution_.end(), comp_pred)).predator * 1.05)};

    std::ofstream gp_script{".tmp.gp"};

    if (!gp_script) {
      throw std::runtime_error{"Impossible to open gnuplot script"};
    }

    gp_script << "set title 'ECOSYSTEM TRAJECTORY'\n"
              << "set xlabel 'prey'\n"
              << "set ylabel 'predator'\n"
              << "set xrange [0:" << xrange << "]\n"
              << "set yrange [0:" << yrange << "]\n"
              << "plot '.tmp.csv' using 1:2 with linespoints pointtype 6 pointsize 0 lc "
              << "rgb 'red' title 'Evolution'\n";

    gp_script.flush();

    if (system("gnuplot -persistent .tmp.gp") != 0) {
      throw std::runtime_error{"An error occured during gnuplot execution"};
    }

    if (std::remove(".tmp.csv") != 0) {  // costruisci fs path e usa quello
      std::cerr << "Failed to delete temporary data file .tmp.csv";
    }
    if (std::remove(".tmp.gp") != 0) {  // costruisci fs path e usa quello
      std::cerr << "Failed to delete temporary data file .tmp.gp";
    }
  }
}

void Simulation::save_trajectory(std::string const& name) const {
  if (evolution_.size() < 2) {
    std::cerr << "No evolution to display.";
  } else {
    std::ofstream data_file{".tmp.csv"};

    if (!data_file) {
      throw std::runtime_error{"Impossible to open output file"};
    }

    for (auto const& s : evolution_) {
      data_file << s.prey << '\t' << s.predator << '\n';
    }

    data_file.flush();

    const double xrange{((*std::max_element(evolution_.begin(), evolution_.end(), comp_prey)).prey * 1.05)};
    const double yrange{((*std::max_element(evolution_.begin(), evolution_.end(), comp_pred)).predator * 1.05)};

    std::ofstream gp_script{".tmp.gp"};

    if (!gp_script) {
      throw std::runtime_error{"Impossible to open gnuplot script"};
    }
    gp_script << "set terminal svg size 800,600 font 'Arial,10' background 'white'\n"
              << "set output '" << name << ".trajectory.svg'\n"
              << "set title 'ECOSYSTEM TRAJECTORY'\n"
              << "set xlabel 'prey'\n"
              << "set ylabel 'predator'\n"
              << "set xrange [0:" << xrange << "]\n"
              << "set yrange [0:" << yrange << "]\n"
              << "plot '.tmp.csv' using 1:2 with linespoints pointtype 6 pointsize 0 lc "
              << "rgb 'red' title 'Evolution'\n";

    gp_script.flush();

    if (system("gnuplot -persistent .tmp.gp") != 0) {
      throw std::runtime_error{"An error occured during gnuplot execution"};
    } else {
      std::cout << "Trajectory saved in " << name << ".svg\n";
    }

    if (std::remove(".tmp.gp") != 0) {
      std::cerr << "Failed to delete gnuplot script .tmp.gp";
    }
  }
}

void Simulation::show_statistics(int p) {
  if (evolution_.size() < 2) {
    std::cerr << "Not enough data for statistics.\n";
  } else {
    this->statistics();
    std::cout << "PREY\' STATISTICS\n"
              << "mean: " << std::setprecision(p) << prey_stat_.mean << "\n"
              << "\u03C3: " << std::setprecision(p) << prey_stat_.sigma << '\n'
              << "maximum: " << std::setprecision(p) << prey_stat_.maximum << '\n'
              << "minimum: " << std::setprecision(p) << prey_stat_.minimum << '\n'
              << "PREDATOR\' STATISTICS\n"
              << "mean: " << std::setprecision(p) << pred_stat_.mean << "\n"
              << "\u03C3: " << std::setprecision(p) << pred_stat_.sigma << '\n'
              << "maximum: " << std::setprecision(p) << pred_stat_.maximum << '\n'
              << "minimum: " << std::setprecision(p) << pred_stat_.minimum << '\n';

    std::ofstream data_file{".tmp.csv"};  // PREPARE DATA TO PLOT

    if (!data_file) {
      throw std::runtime_error{"Impossible to open temporary data file"};
    }

    double step{0};
    for (auto const& s : evolution_) {
      data_file << step << '\t' << s.prey << '\t' << s.predator << '\t' << s.H << '\n';
      step += dt_;
    }

    data_file.flush();

    std::ofstream gp_script{".tmp.gp"};

    if (!gp_script) {
      throw std::runtime_error{"Impossible to open gnuplot script"};
    }
    gp_script << "set multiplot layout 2,1\n"
              << "set title 'POPULATIONS'\n"
              << "set xlabel 'time'\n"
              << "set ylabel '# of individuals'\n"
              << "set xrange [0:" << step << "]\n"
              << "set yrange [" << (std::fmin(prey_stat_.minimum, pred_stat_.minimum) * 0.95) << ":"
              << (std::fmax(prey_stat_.maximum, pred_stat_.maximum) * 1.05) << "]\n"
              << "plot '.tmp.csv' using 1:2 with linespoints pointtype 6 pointsize 0 lc "
              << "rgb 'red' title 'prey', \\\n"
              << "'.tmp.csv' using 1:3 with linespoints pointtype 6 pointsize 0 lc "
              << "rgb 'blue' title 'predator'\n"

              << "set title 'H(t)'\n"
              << "set xlabel 'time'\n"
              << "set ylabel 'H'\n"
              << "set xrange [0:" << step << "]\n"
              << "set yrange ["
              << (*std::min_element(evolution_.begin(), evolution_.end(),
                                    [&](State const& a, State const& b) { return a.H < b.H; }))
                     .H
              << ":"
              << (*std::max_element(evolution_.begin(), evolution_.end(),
                                    [&](State const& a, State const& b) { return a.H < b.H; }))
                     .H
              << "]\n"
              << "plot '.tmp.csv' using 1:4 with linespoints pointtype 6 pointsize 0 lc "
              << "rgb 'green' title 'H(t)'\n"

              << "unset multiplot";

    gp_script.flush();

    if (system("gnuplot -persistent .tmp.gp") != 0) {
      throw std::runtime_error{"An error occured during gnuplot execution"};
    }

    if (std::remove(".tmp.gp") != 0) {
      std::cerr << "Failed to delete temporary data file .tmp.gp";
    }

    if (std::remove(".tmp.csv") != 0) {
      std::cerr << "Failed to delete temporary data file .tmp.csv";
    }
  }
}

void Simulation::save_statistics(int p, std::string const& name) {
  if (evolution_.size() < 2) {
    std::cerr << "Not enough data for statistics.\n";
  } else {
    this->statistics();
    std::ofstream outfile{name + ".statistics.txt"};
    if (!outfile) {
      throw std::runtime_error{"Impossible to open output file"};
    }
    outfile << "PREY\' STATISTICS\n"
            << "- mean: " << std::setprecision(p) << prey_stat_.mean << "\n"
            << "- \u03C3: " << std::setprecision(p) << prey_stat_.sigma << '\n'
            << "- maximum: " << std::setprecision(p) << prey_stat_.maximum << '\n'
            << "- minimum: " << std::setprecision(p) << prey_stat_.minimum << '\n'
            << "PREDATOR\' STATISTICS\n"
            << "- mean: " << std::setprecision(p) << pred_stat_.mean << "\n"
            << "- \u03C3: " << std::setprecision(p) << pred_stat_.sigma << '\n'
            << "- maximum: " << std::setprecision(p) << pred_stat_.maximum << '\n'
            << "- minimum: " << std::setprecision(p) << pred_stat_.minimum << '\n';

    std::ofstream data_file{".tmp.csv"};  // PREPARE DATA TO PLOT

    if (!data_file) {
      throw std::runtime_error{"Impossible to open temporary data file"};
    }

    double step{0};
    for (auto const& s : evolution_) {
      data_file << step << '\t' << s.prey << '\t' << s.predator << '\t' << s.H << '\n';
      step += dt_;
    }

    data_file.flush();

    std::ofstream gp_script{".tmp.gp"};

    if (!gp_script) {
      throw std::runtime_error{"Impossible to open gnuplot script"};
    }
    gp_script << "set terminal svg size 800,600 font 'Arial,10' background 'white'\n"
              << "set output '" << name << ".statistics.svg'\n"

              << "set multiplot layout 2,1\n"

              << "set title 'POPULATIONS'\n"
              << "set xlabel 'time'\n"
              << "set ylabel '# of individuals'\n"
              << "set xrange [0:" << step << "]\n"
              << "set yrange [" << (std::fmin(prey_stat_.minimum, pred_stat_.minimum) * 0.95) << ":"
              << (std::fmax(prey_stat_.maximum, pred_stat_.maximum) * 1.05) << "]\n"
              << "plot '.tmp.csv' using 1:2 with linespoints pointtype 6 pointsize 0 lc "
              << "rgb 'red' title 'prey', \\\n"
              << "'.tmp.csv' using 1:3 with linespoints pointtype 6 pointsize 0 lc "
              << "rgb 'blue' title 'predator'\n"

              << "set title 'H(t)'\n"
              << "set xlabel 'time'\n"
              << "set ylabel 'H'\n"
              << "set xrange [0:" << step << "]\n"
              << "set yrange ["
              << ((*std::min_element(evolution_.begin(), evolution_.end(),
                                     [&](State const& a, State const& b) { return a.H < b.H; }))
                      .H *
                  0.95)
              << ":"
              << ((*std::max_element(evolution_.begin(), evolution_.end(),
                                     [&](State const& a, State const& b) { return a.H < b.H; }))
                      .H *
                  1.05)
              << "]\n"
              << "plot '.tmp.csv' using 1:4 with linespoints pointtype 6 pointsize 0 lc "
              << "rgb 'green' title 'H(t)'\n"

              << "unset multiplot";

    gp_script.flush();

    if (system("gnuplot -persistent .tmp.gp") != 0) {
      throw std::runtime_error{"An error occured during gnuplot execution"};
    } else {
      std::cout << "Statistics saved in " << name << ".txt and in " << name << ".svg\n";
    }

    if (std::remove(".tmp.gp") != 0) {
      std::cerr << "Failed to delete temporary data file .tmp.gp";
    }

    if (std::remove(".tmp.csv") != 0) {
      std::cerr << "Failed to delete temporary data file .tmp.csv";
    }
  }
}

// ###################### OPERATORS

bool operator==(Parameters const& a, Parameters const& b) {
  return (a.A == b.A && a.B == b.B && a.C == b.C && a.D == b.D);
}

bool operator==(State const& a, State const& b) { return (a.prey == b.prey && a.predator == b.predator); }

// ###################### FREE FUNCTIONS

bool comp_prey(State const& a, State const& b) { return a.prey < b.prey; }

bool comp_pred(State const& a, State const& b) { return a.predator < b.predator; }

bool read(std::ifstream& in, std::string& str, char const delimiter) {
  while (std::getline(in, str, delimiter)) {
    if (str.empty() || str[0] == '#') {
      continue;
    }
    return true;
  }
  return false;
}
}  // namespace lv
