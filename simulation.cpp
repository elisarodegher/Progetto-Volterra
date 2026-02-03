#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace volterra {

//------------------------FUNZIONE PRIVATA--------------------

// SET_SIMULATION()
Simulation Simulation::set_simulation() {
  Parameters p{0.0, 0.0, 0.0, 0.0};
  double prey{0.0}, predator{0}, dt{0.0}, it{0.0};

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
      iterations_(it <= 0.0
                      ? throw std::invalid_argument("Errore: input non valido.")
                      : static_cast<std::size_t>(it)) {
  if (prey <= 0 || pred <= 0 || dt <= 0 || p.A <= 0 || p.B <= 0 || p.C <= 0 ||
      p.D < 0) {
    throw std::invalid_argument("Errore: input non valido.");
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
    throw std::runtime_error("\n ---------------ESTINZIONE--------------- \n");
  }

  evolution_.push_back(State{state_.prey * par_.D / par_.C,
                             state_.predator * par_.A / par_.B, state_.H});
}

// GO()
void Simulation::go() {
  while (evolution_.size() < iterations_) {
    try {
      this->evolve();
    } catch (const std::runtime_error& e) {
      std::cerr << e.what();
      break;
    }
  }
}

// SAVE_EVOLUTION()
void Simulation::save_evolution() {
  std::ofstream outfile{"evolution.csv"};
  outfile << "prey\tpredator\tH\n";
  for (auto const& s : evolution_)
    outfile << s.prey << '\t' << s.predator << '\t' << s.H << '\n';
}

// SAVE_PLOT()
void Simulation::save_plot() {
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
      << "set output 'plot.svg'\n"
      << "set title 'POPOLAZIONE'\n"
      << "set xlabel 'TEMPO'\n"
      << "set ylabel 'INDIVIDUI'\n"
      << "set xrange [0:" << step << "]\n"
      << "set autoscale y\n"
      << "set grid\n"
      << "plot '.tmp.csv' using 1:2 with lines lw 2 lc rgb 'green' title "
         "'PREDE', "
         "'.tmp.csv' using 1:3 with lines lw 2 lc rgb 'orange' title "
         "'PREDATORI'\n";
  gp_script.close();

  std::system("gnuplot .tmp.gp");
  std::remove(".tmp.gp");
  std::remove(".tmp.csv");

  std::cout << "\nGrafico ed evoluzione temporale della simulazione sono "
               "stati salvati.\n";
}

// ------------------- FUNZIONI ESTERNE-------------------
// OPERATORE = TRA PARAMETRI PER TEST
bool operator==(Parameters const& a, Parameters const& b) {
  return a.A == b.A && a.B == b.B && a.C == b.C && a.D == b.D;
}
// OPERATORE = TRA STATES PER TEST
bool operator==(State const& a, State const& b) {
  return a.prey == b.prey && a.predator == b.predator;
}
// FUNZIONE DI LETTURA SICURA DI INPUT
double control(const std::string& input) {
  double val;
  std::cout << input;
  if (!(std::cin >> val) || val <= 0) {
    throw std::invalid_argument("Errore: input non valido.\n");
  }
  return val;
}

}  // namespace volterra
