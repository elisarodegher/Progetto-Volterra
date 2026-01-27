#ifndef LV_PROJECT_HPP
#define LV_PROJECT_HPP

#include <cmath>
#include <iostream>
#include <vector>

namespace lv {

struct Parameters {
  double A;  // prey birth rate
  double B;  // prey death rate
  double C;  // predator birth rate
  double D;  // predator death rate
};

struct State {
  double prey;
  double predator;
  double H;
};

struct Statistics {
  double mean;
  double sigma;
  double maximum;
  double minimum;
};

class Simulation {
  Parameters const par_;
  double const dt_;
  std::size_t const iterations_;
  State state_;
  std::vector<State> evolution_;
  Statistics prey_stat_;
  Statistics pred_stat_;

  explicit Simulation(Parameters par, double prey, double pred, double dt, std::size_t it)  // MAIN CONSTRUCTOR
      : par_(par), dt_(dt), iterations_(it) {
    // DEFINING INITIAL STATE
    state_.H = (par_.C * prey) + (par_.B * pred) - ((par_.D * std::log(prey)) + (par_.A * std::log(pred)));

    evolution_.push_back(State{prey, pred, state_.H});

    // SWITCHING TO RELATIVE COORDINATES FOR COMPUTATIONS
    state_.prey = prey * par_.C / par_.D;
    state_.predator = pred * par_.B / par_.A;
  }

  static Simulation set_simulation();

  static Simulation set_simulation(std::string const& file_name);

  static Parameters const& is_good(Parameters const& p);

  static double is_good(double val);

  static double dt_good(double t);

  static std::size_t it_good(double val);

 public:
  // DELEGATING CONSTRUCTORS
  Simulation(Parameters p, double prey, double pred, double t, double n)
      : Simulation(is_good(p), is_good(prey), is_good(pred), dt_good(t), it_good(n)) {}

  Simulation() : Simulation(set_simulation()) {}

  Simulation(std::string const& name) : Simulation(set_simulation(name)) {}

  Parameters const& parameters() const { return par_; }

  State const& init_state() const { return evolution_.front(); }

  State const& current_state() const { return evolution_.back(); }

  State const& internal_state() const { return state_; }

  double dt() const { return dt_; }

  std::size_t iterations() const { return iterations_; }

  std::vector<State> const& evolution() const { return evolution_; }

  Statistics const& prey_stat() { return prey_stat_; }

  Statistics const& pred_stat() { return pred_stat_; }

  void evolve();

  void compute();

  void statistics();

  void show_evolution(int p) const;

  void save_evolution(int p, std::string const& name) const;

  void show_trajectory() const;

  void save_trajectory(std::string const& file_name) const;

  void show_statistics(int p);

  void save_statistics(int p, std::string const& name);
};

// ###################### OPERATORS

bool operator==(Parameters const& a, Parameters const& b);

bool operator==(State const& a, State const& b);

// ###################### FREE FUNCTIONS

bool comp_prey(State const& a, State const& b);

bool comp_pred(State const& a, State const& b);

bool read(std::ifstream& in, std::string& str, char const delimiter);
}  // namespace lv
#endif
