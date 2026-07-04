#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace volterra {

//-------------------------CONSTRUCTORS-------------------------------

// MAIN CONSTRUCTOR
Simulation::Simulation(Parameters p_, double x_, double y_, double dt_,
                       double it_)
    : parameters_(p_), timescale_(dt_), iterations_(it_) {
  if (it_ <= 0.0 || std::floor(it_) != it_ || x_ <= 0 || y_ <= 0 || dt_ <= 0 ||
      p_.a <= 0 || p_.b <= 0 || p_.c <= 0 || p_.d <= 0)
    throw std::invalid_argument("Invalid input.");

  state_.H = (parameters_.c * x_) + (parameters_.b * y_) -
             ((parameters_.d * std::log(x_)) + (parameters_.a * std::log(y_)));

  evolution_.push_back(State{x_, y_, state_.H});

  // Coordinate change
  state_.x = x_ * parameters_.c / parameters_.d;
  state_.y = y_ * parameters_.b / parameters_.a;
}

//--------------------------PUBLIC FUNCTIONS-----------------------

// EVOLVE()
void Simulation::evolve() {
  auto x_rel = state_.x;
  auto y_rel = state_.y;

  auto x = evolution_.back().x;
  auto y = evolution_.back().y;

  state_ =
      State{x_rel + parameters_.a * (1 - y_rel) * x_rel * timescale_,
            y_rel + parameters_.d * (x_rel - 1) * y_rel * timescale_,
            (parameters_.c * x) + (parameters_.b * y) -
                (parameters_.d * std::log(x)) - (parameters_.a * std::log(y))};

  if ((state_.x <= 0) || (state_.y <= 0)) {
    state_ = State{x_rel, y_rel, evolution_.back().H};
    throw std::runtime_error("\nSIMULATION'S OVER: ECOSYSTEM WENT EXTINCT.\n");
  }

  evolution_.push_back(State{state_.x * parameters_.d / parameters_.c,
                             state_.y * parameters_.a / parameters_.b,
                             state_.H});
}

// GO()
void Simulation::go() {
  while (static_cast<double>(evolution_.size()) < iterations_) {
    try {
      this->evolve();
    } catch (const std::runtime_error& error) {
      std::cerr << error.what();
      break;
    }
  }
}

// SAVE_EVOLUTION()
void Simulation::save_evolution() {
  std::ofstream outfile{"EVOLUTION.csv"};
  outfile << "x\ty\tH\n";
  for (auto const& state : evolution_) {
    outfile << state.x << '\t' << state.y << '\t' << state.H << '\n';
  }
}

// SAVE_PLOT()
void Simulation::save_plot() {
  std::ofstream data_file{".tmp.csv"};
  double t{0};

  for (auto const& state : evolution_) {
    data_file << t << '\t' << state.x << '\t' << state.y << '\t' << state.H << '\n';
    t += timescale_;
  }

  data_file.close();

  std::ofstream gp_script{".tmp.gp"};
  gp_script
      << "set terminal svg size 1000,600 font 'Arial,14' background '#99d6e1'\n"
      << "set output 'PLOT.svg'\n"
      << "set title 'POPULATION'\n"
      << "set xlabel 'TIME'\n"
      << "set ylabel 'INDIVIDUALS'\n"
      << "set xrange [0:" << t << "]\n"
      << "set autoscale y\n"
      << "set grid\n"
      << "plot '.tmp.csv' using 1:2 with lines lw 2 lc rgb 'green' title "
         "'PREY', "
      << "'.tmp.csv' using 1:3 with lines lw 2 lc rgb 'orange' title "
         "'PREDATORS'\n";

  gp_script.close();

  int check = std::system("gnuplot .tmp.gp");
  if (check != 0) {
    std::cerr << "gnuplot failed\n";
  }
  std::remove(".tmp.gp");
  std::remove(".tmp.csv");

  std::cout << "\nPlot and temporal evolution of the simulation saved.\n";
}

//--------------------EXTERNAL FUNCTIONS------------------------

// OPERATOR== PARAMETERS
bool operator==(Parameters const& example1, Parameters const& example2) {
  return example1.a == example2.a && example1.b == example2.b &&
         example1.c == example2.c && example1.d == example2.d;
}

// OPERATOR== STATES
bool operator==(State const& example1, State const& example2) {
  return example1.x == example2.x && example1.y == example2.y;
}

// SAFE INPUT FUNCTION
double control(const std::string& message) {
  double input;
  std::cout << message;
  std::cin >> input;
  if (std::cin.fail() || input <= 0) {
    throw std::invalid_argument("Invalid input.\n");
  }
  return input;
}

}  // namespace volterra