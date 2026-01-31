#include "simulation.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>

#include "simulation.hpp"

namespace volterra {

// INIZIALIZZAZIONE DI UNA SIMULAZIONE

volterra::Simulation volterra::Simulation::set_simulation() {
  Parameters p{};
  double prey{}, predator{}, dt{};
  std::size_t n{};  // <-- std::size_t per il numero di iterazioni

  // Funzione helper per leggere un double sicuro
  auto read_double = [](const std::string& prompt) -> double {
    double val;
    while (true) {
      std::cout << prompt;
      if (std::cin >> val) break;
      std::cout << "Input non valido, riprova.\n";
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return val;
  };

  // Funzione helper per leggere un size_t sicuro
  auto read_size_t = [](const std::string& prompt) -> std::size_t {
    std::size_t val;
    while (true) {
      std::cout << prompt;
      if (std::cin >> val) break;
      std::cout << "Input non valido, riprova.\n";
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return val;
  };

  // Chiediamo i parametri uno per uno
  p.A = read_double("Inserisci tasso di natalità prede (A): ");
  p.B = read_double("Inserisci tasso di mortalità prede (B): ");
  p.C = read_double("Inserisci tasso di natalità predatori (C): ");
  p.D = read_double("Inserisci tasso di mortalità predatori (D): ");

  prey = read_double("Inserisci numero iniziale di prede: ");
  predator = read_double("Inserisci numero iniziale di predatori: ");
  dt = read_double("Inserisci passo di discretizzazione (dt): ");
  n = read_size_t("Inserisci numero di iterazioni: ");

  // Chiamata corretta al costruttore con ordine e tipi esatti
  return Simulation(p, prey, predator, dt, n);
}

// FUNZIONI DI CONTROLLO

Parameters const& Simulation::is_good(Parameters const& p) {
  if (!((p.A > 0) && (p.B > 0) && (p.C > 0) && (p.D > 0))) {
    throw std::invalid_argument{
        "Parameters must be strictly greater than zero"};
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
    std::clog << "Quality-Warning: A large time increment produces less stable "
                 "simulations\n";
  }
  return t;
}

std::size_t Simulation::it_good(double const val) {
  if (!(val > 0)) {
    throw std::invalid_argument{"Iterations should really be a natural number"};
  }
  return static_cast<std::size_t>(val);
}

// EVOLUZIONE

void Simulation::evolve() {
  auto x = evolution_.back().prey;
  auto y = evolution_.back().predator;

  assert((0 < x && 0 < y));

  auto x_rel = state_.prey;
  auto y_rel = state_.predator;

  state_ = State{x_rel + par_.A * (1 - y_rel) * x_rel * dt_,
                 y_rel + par_.D * (x_rel - 1) * y_rel * dt_,
                 (par_.C * x) + (par_.B * y) - (par_.D * std::log(x)) -
                     (par_.A * std::log(y))};

  if (!(state_.prey > 0) || !(state_.predator > 0)) {
    state_ = State{x_rel, y_rel, evolution_.back().H};
    throw std::runtime_error{
        "Simulation is over: one species has reached extinction\n"};
  } else {
    evolution_.push_back(State{state_.prey * par_.D / par_.C,
                               state_.predator * par_.A / par_.B, state_.H});
  }
}

// RIPETE EVOLVE FINO A N ITERAZIONI o estinzione
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

// CALCOLA LE STATISTICHE
void Simulation::statistics() {
  struct Sums {
    double sum_prey;
    double sum_pred;
    double sum_prey2;
    double sum_pred2;
  };

  auto begin = evolution_.begin();
  auto end = evolution_.end();

  Sums sum =
      std::accumulate(begin, end, Sums{0., 0., 0., 0.}, [](Sums s, State p) {
        return Sums{(s.sum_prey + p.prey), (s.sum_pred + p.predator),
                    (s.sum_prey2 + p.prey * p.prey),
                    (s.sum_pred2 + p.predator * p.predator)};
      });

  double N{static_cast<double>(evolution_.size())};
  double mean{sum.sum_prey / N};

  prey_stat_ =
      Statistics{mean, std::sqrt(sum.sum_prey2 / N - std::pow(mean, 2.)),
                 (*std::max_element(begin, end, comp_prey)).prey,
                 (*std::min_element(begin, end, comp_prey)).prey};

  mean = sum.sum_pred / N;

  pred_stat_ =
      Statistics{mean, std::sqrt(sum.sum_pred2 / N - std::pow(mean, 2.)),
                 (*std::max_element(begin, end, comp_pred)).predator,
                 (*std::min_element(begin, end, comp_pred)).predator};
}

// SALVA L'EVOLUZIONE IN UN FILE
void Simulation::save_evolution(std::string const& name) const {
  if (evolution_.size() < 2) {
    std::cerr << "No evolution to display.";
  } else {
    std::ofstream outfile{name + ".evolution.csv"};
    if (!outfile) {
      throw std::runtime_error{"Impossible to open output file"};
    }
    outfile << "prey\tpredator\tH\n";
    for (auto const& s : evolution_) {
      outfile << s.prey << '\t' << s.predator << '\t' << s.H << '\n';
    }
    std::cout << "Evolution saved in " << name << ".evolution.csv\n";
  }
}

// SALVA LE ORBITE SU UN FILE
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

    const double xrange{
        ((*std::max_element(evolution_.begin(), evolution_.end(), comp_prey))
             .prey *
         1.05)};
    const double yrange{
        ((*std::max_element(evolution_.begin(), evolution_.end(), comp_pred))
             .predator *
         1.05)};

    std::ofstream gp_script{".tmp.gp"};

    if (!gp_script) {
      throw std::runtime_error{"Impossible to open gnuplot script"};
    }
    gp_script
        << "set terminal svg size 800,600 font 'Arial,10' background 'white'\n"
        << "set output '" << name << ".trajectory.svg'\n"
        << "set title 'ECOSYSTEM TRAJECTORY'\n"
        << "set xlabel 'prey'\n"
        << "set ylabel 'predator'\n"
        << "set xrange [0:" << xrange << "]\n"
        << "set yrange [0:" << yrange << "]\n"
        << "plot '.tmp.csv' using 1:2 with linespoints pointtype 6 pointsize 0 "
           "lc "
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

// SALVA LE STATISTICHE IN UN FILE

void Simulation::save_statistics(std::string const& name) {
  if (evolution_.size() < 2) {
    std::cerr << "Not enough data for statistics.\n";
  } else {
    this->statistics();

    std::ofstream outfile{name + ".statistics.txt"};
    if (!outfile) {
      throw std::runtime_error{"Impossible to open output file"};
    }

    outfile << "PREY' STATISTICS\n"
            << "- mean: " << prey_stat_.mean << "\n"
            << "- σ: " << prey_stat_.sigma << '\n'
            << "- maximum: " << prey_stat_.maximum << '\n'
            << "- minimum: " << prey_stat_.minimum << '\n'
            << "PREDATOR' STATISTICS\n"
            << "- mean: " << pred_stat_.mean << "\n"
            << "- σ: " << pred_stat_.sigma << '\n'
            << "- maximum: " << pred_stat_.maximum << '\n'
            << "- minimum: " << pred_stat_.minimum << '\n';

    std::ofstream data_file{".tmp.csv"};  // PREPARE DATA TO PLOT
    if (!data_file) {
      throw std::runtime_error{"Impossible to open temporary data file"};
    }

    double step{0};
    for (auto const& s : evolution_) {
      data_file << step << '\t' << s.prey << '\t' << s.predator << '\t' << s.H
                << '\n';
      step += dt_;
    }

    data_file.flush();

    std::ofstream gp_script{".tmp.gp"};
    if (!gp_script) {
      throw std::runtime_error{"Impossible to open gnuplot script"};
    }

    gp_script
        << "set terminal svg size 800,600 font 'Arial,10' background 'white'\n"
        << "set output '" << name << ".statistics.svg'\n"
        << "set multiplot layout 2,1\n"

        << "set title 'POPULATIONS'\n"
        << "set xlabel 'time'\n"
        << "set ylabel '# of individuals'\n"
        << "set xrange [0:" << step << "]\n"
        << "set yrange ["
        << (std::fmin(prey_stat_.minimum, pred_stat_.minimum) * 0.95) << ":"
        << (std::fmax(prey_stat_.maximum, pred_stat_.maximum) * 1.05) << "]\n"
        << "plot '.tmp.csv' using 1:2 with linespoints pointtype 6 pointsize 0 "
           "lc "
           "rgb 'red' title 'prey', \\\n"
        << "'.tmp.csv' using 1:3 with linespoints pointtype 6 pointsize 0 lc "
           "rgb 'blue' title 'predator'\n"

        << "set title 'H(t)'\n"
        << "set xlabel 'time'\n"
        << "set ylabel 'H'\n"
        << "set xrange [0:" << step << "]\n"
        << "set yrange ["
        << ((*std::min_element(
                 evolution_.begin(), evolution_.end(),
                 [&](State const& a, State const& b) { return a.H < b.H; }))
                .H *
            0.95)
        << ":"
        << ((*std::max_element(
                 evolution_.begin(), evolution_.end(),
                 [&](State const& a, State const& b) { return a.H < b.H; }))
                .H *
            1.05)
        << "]\n"
        << "plot '.tmp.csv' using 1:4 with linespoints pointtype 6 pointsize 0 "
           "lc "
           "rgb 'green' title 'H(t)'\n"

        << "unset multiplot";

    gp_script.flush();

    if (system("gnuplot -persistent .tmp.gp") != 0) {
      throw std::runtime_error{"An error occured during gnuplot execution"};
    } else {
      std::cout << "Statistics saved in " << name << ".txt and in " << name
                << ".svg\n";
    }

    if (std::remove(".tmp.gp") != 0) {
      std::cerr << "Failed to delete temporary data file .tmp.gp";
    }

    if (std::remove(".tmp.csv") != 0) {
      std::cerr << "Failed to delete temporary data file .tmp.csv";
    }
  }
}

// OPERATORI

bool operator==(Parameters const& a, Parameters const& b) {
  return (a.A == b.A && a.B == b.B && a.C == b.C && a.D == b.D);
}

bool operator==(State const& a, State const& b) {
  return (a.prey == b.prey && a.predator == b.predator);
}

// FUNZIONI ESTERNE

bool comp_prey(State const& a, State const& b) { return a.prey < b.prey; }

bool comp_pred(State const& a, State const& b) {
  return a.predator < b.predator;
}

bool read(std::ifstream& in, std::string& str, char const delimiter) {
  while (std::getline(in, str, delimiter)) {
    if (str.empty() || str[0] == '#') {
      continue;
    }
    return true;
  }
  return false;
}
}  // namespace volterra
