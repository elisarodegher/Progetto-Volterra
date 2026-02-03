#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace volterra {

//------------------------FUNZIONE PRIVATA-------------------

// SET_SIMULATION()
Simulation Simulation::set_simulation() {
  Parameters p{};
  double prey{}, predator{}, dt{}, it{};

  try {
    p.A = control("Inserisci tasso di natalità prede: ");
    p.B = control("Inserisci tasso di mortalità prede: ");
    p.C = control("Inserisci tasso di natalità predatori: ");
    p.D = control("Inserisci tasso di mortalità predatori: ");
    prey = control("Inserisci numero iniziale di prede: ");
    predator = control("Inserisci numero iniziale di predatori: ");
    dt = control("Inserisci l'unità temporale dt: ");
    it = control("Inserisci numero di iterazioni: ");
  } catch (std::invalid_argument const& e) {
    std::cerr << e.what();
    std::exit(EXIT_FAILURE);
  }

  return Simulation(p, prey, predator, dt, it);
}

//-------------------------COSTRUTTORI-------------------------------

// COSTRUTTORE PRINCIPALE
Simulation::Simulation(Parameters p, double prey, double pred, double dt,
                       double it)
    : par_(p),
      dt_(dt),
      iterations_(it < 0.0 ? 0 : static_cast<std::size_t>(it)) {
  if (prey <= 0 || pred <= 0) {
    throw std::invalid_argument("Input non valido.");
  }
  if (dt <= 0) {
    throw std::invalid_argument("Input non valido.");
  }
  if (iterations_ == 0) {
    throw std::invalid_argument("Input non valido.");
  }

  state_.H = (par_.C * prey) + (par_.B * pred) -
             ((par_.D * std::log(prey)) + (par_.A * std::log(pred)));
  evolution_.push_back(State{prey, pred, state_.H});

  // Cambio di coordinate
  state_.prey = prey * par_.C / par_.D;
  state_.predator = pred * par_.B / par_.A;
}

// COSTRUTTORE DI DEFAULT
Simulation::Simulation() : Simulation(set_simulation()) {}

//--------------------------FUNZIONI PUBBLICHE-----------------------

// EVOLVE()
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
    throw std::runtime_error("\n ----------ESTINZIONE--------- \n");
  }

  evolution_.push_back(State{state_.prey * par_.D / par_.C,
                             state_.predator * par_.A / par_.B, state_.H});
}

// COMPUTE()
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

// STATITICS()
void Simulation::statistics() {
  double sum_prey{0.0}, sum_pred{0.0};
  double sum_prey2{0.0}, sum_pred2{0.0};

  for (const auto& s : evolution_) {
    sum_prey += s.prey;
    sum_pred += s.predator;
    sum_prey2 += s.prey * s.prey;
    sum_pred2 += s.predator * s.predator;
  }

  double N = static_cast<double>(evolution_.size());
  double prey_mean = sum_prey / N;
  double pred_mean = sum_pred / N;
  double prey_sigma = std::sqrt(sum_prey2 / N - prey_mean * prey_mean);
  double pred_sigma = std::sqrt(sum_pred2 / N - pred_mean * pred_mean);

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

// SAVE_EVOLUTION()
void Simulation::save_evolution(std::string const& name) const {
  std::ofstream outfile{name + ".evolution.csv"};
  outfile << "prey\tpredator\tH\n";
  for (auto const& s : evolution_)
    outfile << s.prey << '\t' << s.predator << '\t' << s.H << '\n';
}

// SAVE_RESULTS()
void Simulation::save_results(std::string const& name) {
  this->statistics();

  std::cout << "\nStatistiche della simulazione:\n"
            << "\nPREDE\n"
            << "- media: " << prey_stat_.mean << "\n"
            << "- dev std: " << prey_stat_.sigma << "\n"
            << "- max: " << prey_stat_.maximum << "\n"
            << "- min: " << prey_stat_.minimum << "\n"
            << "\nPREDATORI\n"
            << "- media: " << pred_stat_.mean << "\n"
            << "- dev std: " << pred_stat_.sigma << "\n"
            << "- max: " << pred_stat_.maximum << "\n"
            << "- min: " << pred_stat_.minimum << "\n"
            << "\n\nGrafico ed evoluzione temporale della simulazione sono "
               "stati salvati.\n";

  std::ofstream data_file{".tmp.csv"};
  double step{0};
  for (auto const& s : evolution_) {
    data_file << step << '\t' << s.prey << '\t' << s.predator << '\t' << s.H
              << '\n';
    step += dt_;
  }
  data_file.close();

  std::ofstream gp_script{".tmp.gp"};
  gp_script
      << "set terminal svg size 1000,600 font 'Arial,14' background '#99d6e1'\n"
      << "set output '" << name << ".plot.svg'\n"
      << "set title 'POPOLAZIONE'\n"
      << "set xlabel 'TEMPO'\n"
      << "set ylabel 'INDIVIDUI'\n"
      << "set xrange [0:" << step << "]\n"
      << "set yrange ["
      << std::fmin(prey_stat_.minimum, pred_stat_.minimum) * 0.9 << ":"
      << std::fmax(prey_stat_.maximum, pred_stat_.maximum) * 1.1 << "]\n"
      << "set grid\n"
      << "plot '.tmp.csv' using 1:2 with lines lw 2 lc rgb 'green' title "
         "'PREDE', "
         "'.tmp.csv' using 1:3 with lines lw 2 lc rgb 'orange' title "
         "'PREDATORI'\n";
  gp_script.close();

  std::system("gnuplot .tmp.gp");
  std::remove(".tmp.gp");
  std::remove(".tmp.csv");
}

// ------------------- FUNZIONI ESTERNE-------------------
// OPERATORE = TRA PARAMETRI
bool operator==(Parameters const& a, Parameters const& b) {
  return a.A == b.A && a.B == b.B && a.C == b.C && a.D == b.D;
}
// OPERATORE = TRA STATES
bool operator==(State const& a, State const& b) {
  return a.prey == b.prey && a.predator == b.predator;
}
// FUNZIONI DI PARAGONE
bool comp_prey(State const& a, State const& b) { return a.prey < b.prey; }
bool comp_pred(State const& a, State const& b) {
  return a.predator < b.predator;
}
// FUNZIONE DI LETTURA SICURA DI IMPUT
double control(const std::string& input) {
  double val;
  std::cout << input;
  if (!(std::cin >> val) || val <= 0) {
    throw std::invalid_argument("Errore: input non valido.\n");
  }
  return val;
}

}  // namespace volterra
