#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>

namespace volterra {

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

double read_double(const std::string& input) {
  double val;
  std::cout << input;
  if (!(std::cin >> val)) {
    throw std::invalid_argument("Errore: input non valido.\n");
  }
  return val;
}

std::size_t read_size_t(const std::string& prompt) {
  std::string s;
  std::cout << prompt;
  std::cin >> s;

  unsigned long long v = std::stoull(s);
  if (std::to_string(v) != s)
    throw std::invalid_argument("Errore: input non valido");

  return v;
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

  // Cambio di coordinate
  state_.prey = prey * par_.C / par_.D;
  state_.predator = pred * par_.B / par_.A;
}

Simulation::Simulation() : Simulation(set_simulation()) {}

// ------------------- inizializzazione di una simulazione -------------------

Simulation Simulation::set_simulation() {
  Parameters p{};
  double prey{}, predator{}, dt{};
  std::size_t n{};

  try {
    p.A = control(read_double("Inserisci tasso di natalità prede: "));
    p.B = control(read_double("Inserisci tasso di mortalità prede: "));
    p.C = control(read_double("Inserisci tasso di natalità predatori: "));
    p.D = control(read_double("Inserisci tasso di mortalità predatori: "));
    prey = control(read_double("Inserisci numero iniziale di prede: "));
    predator = control(read_double("Inserisci numero iniziale di predatori: "));
    dt = control(read_double("Inserisci l'unità temporale dt: "));
    n = control(read_size_t("Inserisci numero di iterazioni: "));
  } catch (std::invalid_argument const& e) {
    std::cerr << e.what();
    std::exit(EXIT_FAILURE);
  }

  return Simulation(p, prey, predator, dt, n);
}

// ------------------- EVOLUZIONE -------------------

// evolve: descrive l'iterazione
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
    throw std::runtime_error("\n ESTINZIONE :( \n");
  } else {
    evolution_.push_back(State{state_.prey * par_.D / par_.C,
                               state_.predator * par_.A / par_.B, state_.H});
  }
}

// compute: evolve n volte
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
  double sum_prey = 0.0;
  double sum_pred = 0.0;
  double sum_prey2 = 0.0;
  double sum_pred2 = 0.0;

  for (const auto& s : evolution_) {
    sum_prey += s.prey;
    sum_pred += s.predator;
    sum_prey2 += s.prey * s.prey;
    sum_pred2 += s.predator * s.predator;
  }

  const double N = static_cast<double>(evolution_.size());

  const double prey_mean = sum_prey / N;
  const double pred_mean = sum_pred / N;
  const double prey_sigma = std::sqrt(sum_prey2 / N - prey_mean * prey_mean);
  const double pred_sigma = std::sqrt(sum_pred2 / N - pred_mean * pred_mean);

  prey_stat_ = Statistics{
      prey_mean, prey_sigma,
      std::max_element(evolution_.begin(), evolution_.end(), comp_prey)->prey,
      std::min_element(evolution_.begin(), evolution_.end(), comp_prey)->prey};

  pred_stat_ = Statistics{
      pred_mean, pred_sigma,
      std::max_element(evolution_.begin(), evolution_.end(), comp_pred)
          ->predator,
      std::min_element(evolution_.begin(), evolution_.end(), comp_pred)
          ->predator};
}

// ------------------- RISULTATI -------------------

void Simulation::save_evolution(std::string const& name) const {
  std::ofstream outfile{name + ".evolution.csv"};
  outfile << "prey\tpredator\tH\n";
  for (auto const& s : evolution_)
    outfile << s.prey << '\t' << s.predator << '\t' << s.H << '\n';
}

void Simulation::save_statistics(std::string const& name) {
  this->statistics();

  std::cout << "\n Due nuovi file sono stati salvati in /build. Statistiche "
               "della simulazione:\n "
            << "\nPREDA\n"
            << "- media: " << prey_stat_.mean << "\n"
            << "- dev std: " << prey_stat_.sigma << "\n"
            << "- max: " << prey_stat_.maximum << "\n"
            << "- min: " << prey_stat_.minimum << "\n"
            << "PREDATORE\n"
            << "- media: " << pred_stat_.mean << "\n"
            << "- dev std: " << pred_stat_.sigma << "\n"
            << "- max: " << pred_stat_.maximum << "\n"
            << "- min: " << pred_stat_.minimum << "\n";

  // --- dati per il grafico ---
  std::ofstream data_file{".tmp.csv"};
  double step{0};
  for (auto const& s : evolution_) {
    data_file << step << '\t' << s.prey << '\t' << s.predator << '\t' << s.H
              << '\n';
    step += dt_;
  }
  data_file.close();

  // --- script gnuplot ---
  std::ofstream gp_script{".tmp.gp"};
  gp_script
      << "set terminal svg size 1000,600 font 'Arial,14' background '#99d6e1'\n"
      << "set output '" << name << ".statistics.svg'\n"
      << "set title 'POPOLAZIONE' font ',16'\n"
      << "set xlabel 'TEMPO' font ',14'\n"
      << "set ylabel 'INDIVIDUI' font ',14'\n"
      << "set xrange [0:" << step << "]\n"
      << "set yrange ["
      << std::fmin(prey_stat_.minimum, pred_stat_.minimum) * 0.9 << ":"
      << std::fmax(prey_stat_.maximum, pred_stat_.maximum) * 1.1 << "]\n"
      << "set grid lw 1 lc rgb '#dddddd'\n"
      << "plot '.tmp.csv' using 1:2 with lines lw 2 lc rgb 'green' title "
         "'POPOLAZIONE DI PREDE', "
         "'.tmp.csv' using 1:3 with lines lw 2 lc rgb 'orange' title "
         "'POPOLAZIONE DI PREDATORI'\n";
  gp_script.close();

  // --- esegue gnuplot per generare l'SVG ---
  std::system("gnuplot .tmp.gp");

  // --- pulizia ---
  std::remove(".tmp.gp");
  std::remove(".tmp.csv");
}

}  // namespace volterra