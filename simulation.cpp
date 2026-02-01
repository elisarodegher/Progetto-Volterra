#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>

namespace volterra {

// ------------------- FUNZIONI LIBERE -------------------

double read_double(const std::string& input) {
  double val;
  std::cout << input;
  if (!(std::cin >> val)) {
    throw std::invalid_argument("Errore: input non valido.\n");
  }
  return val;
}

std::size_t read_size_t(const std::string& input) {
  std::size_t val;
  std::cout << input;
  if (!(std::cin >> val)) {
    throw std::invalid_argument("Errore: input non valido.\n");
  }
  return val;
}

// ------------------- FUNZIONI DI CONTROLLO -------------------

Parameters const& Simulation::control(Parameters const& p) {
  if (!(p.A > 0 && p.B > 0 && p.C > 0 && p.D > 0))
    throw std::invalid_argument("Errore: valore non valido.\n");
  return p;
}

double Simulation::control(double val) {
  if (!(val > 0)) throw std::invalid_argument("Errore: valore non valido.\n");
  return val;
}

std::size_t Simulation::control(std::size_t n) {
  if (n == 0) throw std::invalid_argument("Errore: valore non valido.\n");
  return n;
}

// ------------------- COSTRUTTORI -------------------

Simulation::Simulation(Parameters p, double prey, double pred, double dt,
                       std::size_t it)
    : par_(control(p)), dt_(control(dt)), iterations_(control(it)) {
  prey = control(prey);
  pred = control(pred);
  state_.H = (par_.C * prey) + (par_.B * pred) -
             ((par_.D * std::log(prey)) + (par_.A * std::log(pred)));
  evolution_.push_back(State{prey, pred, state_.H});

  // Cambio di coordinate (manteniamo invarianti per la simulazione)
  state_.prey = prey * par_.C / par_.D;
  state_.predator = pred * par_.B / par_.A;
}

Simulation::Simulation() : Simulation(set_simulation()) {}

// ------------------- FUNZIONE DI SETUP -------------------

Simulation Simulation::set_simulation() {
  Parameters p{};
  double prey{}, predator{}, dt{};
  std::size_t n{};

  try {
    p.A = control(read_double("Inserisci tasso di natalità prede (A): "));
    p.B = control(read_double("Inserisci tasso di mortalità prede (B): "));
    p.C = control(read_double("Inserisci tasso di natalità predatori (C): "));
    p.D = control(read_double("Inserisci tasso di mortalità predatori (D): "));
    prey = control(read_double("Inserisci numero iniziale di prede: "));
    predator = control(read_double("Inserisci numero iniziale di predatori: "));
    dt = control(read_double("Inserisci passo di discretizzazione (dt): "));
    n = control(read_size_t("Inserisci numero di iterazioni: "));
  } catch (std::invalid_argument const& e) {
    std::cerr << e.what();
    std::exit(EXIT_FAILURE);
  }

  return Simulation(p, prey, predator, dt, n);
}

// ------------------- EVOLUZIONE -------------------

void Simulation::evolve() {
  auto x_rel = state_.prey;
  auto y_rel = state_.predator;

  auto x = evolution_.back().prey;
  auto y = evolution_.back().predator;

  state_ = State{x_rel + par_.A * (1 - y_rel) * x_rel * dt_,
                 y_rel + par_.D * (x_rel - 1) * y_rel * dt_,
                 (par_.C * x) + (par_.B * y) - (par_.D * std::log(x)) -
                     (par_.A * std::log(y))};

  if (!(state_.prey > 0) || !(state_.predator > 0)) {
    state_ = State{x_rel, y_rel, evolution_.back().H};
    throw std::runtime_error(
        "Simulation is over: one species has reached extinction\n");
  } else {
    evolution_.push_back(State{state_.prey * par_.D / par_.C,
                               state_.predator * par_.A / par_.B, state_.H});
  }
}

void Simulation::compute() {
  while (evolution_.size() < iterations_) {
    try {
      this->evolve();
    } catch (const std::runtime_error& e) {
      std::cerr << e.what();
      break;
    }
  }
}

// ------------------- STATISTICHE -------------------

void Simulation::statistics() {
  struct Sums {
    double sum_prey{0.0}, sum_pred{0.0}, sum_prey2{0.0}, sum_pred2{0.0};
  };

  Sums sum = std::accumulate(evolution_.begin(), evolution_.end(), Sums{},
                             [](Sums s, State const& st) {
                               s.sum_prey += st.prey;
                               s.sum_pred += st.predator;
                               s.sum_prey2 += st.prey * st.prey;
                               s.sum_pred2 += st.predator * st.predator;
                               return s;
                             });

  double N = static_cast<double>(evolution_.size());
  double mean = sum.sum_prey / N;
  prey_stat_ = Statistics{
      mean, std::sqrt(sum.sum_prey2 / N - mean * mean),
      (*std::max_element(evolution_.begin(), evolution_.end(), comp_prey)).prey,
      (*std::min_element(evolution_.begin(), evolution_.end(), comp_prey))
          .prey};

  mean = sum.sum_pred / N;
  pred_stat_ = Statistics{
      mean, std::sqrt(sum.sum_pred2 / N - mean * mean),
      (*std::max_element(evolution_.begin(), evolution_.end(), comp_pred))
          .predator,
      (*std::min_element(evolution_.begin(), evolution_.end(), comp_pred))
          .predator};
}

// ------------------- SALVATAGGIO -------------------

void Simulation::save_evolution(std::string const& name) const {
  if (evolution_.size() < 2) {
    std::cerr << "No evolution to display.\n";
    return;
  }
  std::ofstream outfile{name + ".evolution.csv"};
  if (!outfile) throw std::runtime_error("Impossible to open output file\n");

  outfile << "prey\tpredator\tH\n";
  for (auto const& s : evolution_)
    outfile << s.prey << '\t' << s.predator << '\t' << s.H << '\n';

  std::cout << "Evolution saved in " << name << ".evolution.csv\n";
}

void Simulation::save_trajectory(std::string const& name) const {
  if (evolution_.size() < 2) {
    std::cerr << "No evolution to display.\n";
    return;
  }

  std::ofstream data_file{".tmp.csv"};
  if (!data_file)
    throw std::runtime_error("Impossible to open temporary data file\n");

  for (auto const& s : evolution_)
    data_file << s.prey << '\t' << s.predator << '\n';

  data_file.flush();

  const double xrange =
      (*std::max_element(evolution_.begin(), evolution_.end(), comp_prey))
          .prey *
      1.05;
  const double yrange =
      (*std::max_element(evolution_.begin(), evolution_.end(), comp_pred))
          .predator *
      1.05;

  std::ofstream gp_script{".tmp.gp"};
  if (!gp_script)
    throw std::runtime_error("Impossible to open gnuplot script\n");

  gp_script
      << "set terminal svg size 800,600 font 'Arial,10' background 'white'\n"
      << "set output '" << name << ".trajectory.svg'\n"
      << "set title 'ECOSYSTEM TRAJECTORY'\n"
      << "set xlabel 'prey'\nset ylabel 'predator'\n"
      << "set xrange [0:" << xrange << "]\nset yrange [0:" << yrange << "]\n"
      << "plot '.tmp.csv' using 1:2 with linespoints pointtype 6 pointsize 0 "
         "lc rgb 'red' title 'Evolution'\n";

  gp_script.flush();

  if (system("gnuplot -persistent .tmp.gp") != 0)
    throw std::runtime_error("Errore durante l'esecuzione di gnuplot\n");

  if (std::remove(".tmp.gp") != 0) std::cerr << "Failed to delete .tmp.gp\n";
  if (std::remove(".tmp.csv") != 0) std::cerr << "Failed to delete .tmp.csv\n";

  std::cout << "Trajectory saved in " << name << ".svg\n";
}

void Simulation::save_statistics(std::string const& name) {
  if (evolution_.size() < 2) {
    std::cerr << "Not enough data for statistics.\n";
    return;
  }

  this->statistics();

  std::ofstream outfile{name + ".statistics.txt"};
  if (!outfile) throw std::runtime_error("Impossible to open output file\n");

  outfile << "PREY' STATISTICS\n"
          << "- mean: " << prey_stat_.mean << "\n"
          << "- sigma: " << prey_stat_.sigma << "\n"
          << "- max: " << prey_stat_.maximum << "\n"
          << "- min: " << prey_stat_.minimum << "\n"
          << "PREDATOR' STATISTICS\n"
          << "- mean: " << pred_stat_.mean << "\n"
          << "- sigma: " << pred_stat_.sigma << "\n"
          << "- max: " << pred_stat_.maximum << "\n"
          << "- min: " << pred_stat_.minimum << "\n";

  std::ofstream data_file{".tmp.csv"};
  double step{0};
  for (auto const& s : evolution_) {
    data_file << step << '\t' << s.prey << '\t' << s.predator << '\t' << s.H
              << '\n';
    step += dt_;
  }
  data_file.flush();

  std::ofstream gp_script{".tmp.gp"};
  gp_script
      << "set terminal svg size 800,600 font 'Arial,10' background 'white'\n"
      << "set output '" << name << ".statistics.svg'\n"
      << "set multiplot layout 2,1\n"
      << "set title 'POPULATIONS'\nset xlabel 'time'\nset ylabel '# of "
         "individuals'\n"
      << "set xrange [0:" << step << "]\n"
      << "set yrange ["
      << std::fmin(prey_stat_.minimum, pred_stat_.minimum) * 0.95 << ":"
      << std::fmax(prey_stat_.maximum, pred_stat_.maximum) * 1.05 << "]\n"
      << "plot '.tmp.csv' using 1:2 with linespoints pointtype 6 pointsize 0 "
         "lc rgb 'red' title 'prey',\\\n"
      << "'.tmp.csv' using 1:3 with linespoints pointtype 6 pointsize 0 lc rgb "
         "'blue' title 'predator'\n"
      << "set title 'H(t)'\nset xlabel 'time'\nset ylabel 'H'\n"
      << "set xrange [0:" << step << "]\n"
      << "set yrange ["
      << ((*std::min_element(
               evolution_.begin(), evolution_.end(),
               [](State const& a, State const& b) { return a.H < b.H; }))
              .H *
          0.95)
      << ":"
      << ((*std::max_element(
               evolution_.begin(), evolution_.end(),
               [](State const& a, State const& b) { return a.H < b.H; }))
              .H *
          1.05)
      << "]\n"
      << "plot '.tmp.csv' using 1:4 with linespoints pointtype 6 pointsize 0 "
         "lc rgb 'green' title 'H(t)'\n"
      << "unset multiplot";
  gp_script.flush();

  if (system("gnuplot -persistent .tmp.gp") != 0)
    throw std::runtime_error("Errore durante l'esecuzione di gnuplot\n");

  if (std::remove(".tmp.gp") != 0) std::cerr << "Failed to delete .tmp.gp\n";
  if (std::remove(".tmp.csv") != 0) std::cerr << "Failed to delete .tmp.csv\n";

  std::cout << "Statistics saved in " << name << ".txt and " << name
            << ".svg\n";
}

// ------------------- OPERATORI -------------------

bool operator==(Parameters const& a, Parameters const& b) {
  return a.A == b.A && a.B == b.B && a.C == b.C && a.D == b.D;
}

bool operator==(State const& a, State const& b) {
  return a.prey == b.prey && a.predator == b.predator;
}

// ------------------- FUNZIONI LIBERE -------------------

bool comp_prey(State const& a, State const& b) { return a.prey < b.prey; }
bool comp_pred(State const& a, State const& b) {
  return a.predator < b.predator;
}

bool read(std::ifstream& in, std::string& str, char const delimiter) {
  while (std::getline(in, str, delimiter)) {
    if (str.empty() || str[0] == '#') continue;
    return true;
  }
  return false;
}

}  // namespace volterra