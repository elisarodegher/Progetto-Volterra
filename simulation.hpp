#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace volterra {

// --------------------------- STRUCT ---------------------------

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

// --------------------------- CLASS ---------------------------

class Simulation {
  Parameters const par_;
  double const dt_;
  std::size_t const iterations_;
  State state_;
  std::vector<State> evolution_;
  Statistics prey_stat_;
  Statistics pred_stat_;

  // Funzione che legge dati da input
  static Simulation set_simulation();

  // Funzioni di controllo dei valori
  static Parameters const& control(Parameters const& p);
  static double control(double val);
  static std::size_t control(std::size_t n);

 public:
  // ------------------- COSTRUTTORI -------------------
  explicit Simulation(Parameters p, double prey, double pred, double dt,
                      std::size_t it);
  Simulation();  // usa set_simulation()

  // ------------------- GETTER -------------------
  Parameters const& parameters() const { return par_; }
  State const& init_state() const { return evolution_.front(); }
  State const& current_state() const { return evolution_.back(); }
  State const& internal_state() const { return state_; }
  double dt() const { return dt_; }
  std::size_t iterations() const { return iterations_; }
  std::vector<State> const& evolution() const { return evolution_; }
  Statistics const& prey_stat() const { return prey_stat_; }
  Statistics const& pred_stat() const { return pred_stat_; }

  // ------------------- CALCOLI -------------------
  void evolve();
  void compute();
  void statistics();

  // ------------------- OUTPUT -------------------
  void save_evolution(std::string const& name) const;
  void save_statistics(std::string const& name);
};

// --------------------------- OPERATORI ---------------------------
bool operator==(Parameters const& a, Parameters const& b);
bool operator==(State const& a, State const& b);

// --------------------------- FUNZIONI LIBERE ---------------------------
bool comp_prey(State const& a, State const& b);
bool comp_pred(State const& a, State const& b);
bool read(std::ifstream& in, std::string& str, char const delimiter);

double read_double(const std::string& input);
std::size_t read_size_t(const std::string& input);

}  // namespace volterra

#endif
