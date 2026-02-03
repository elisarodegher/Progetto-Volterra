#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace volterra {

// --------------------------- STRUCT ---------------------------

struct Parameters {
  double A;  // tasso di natalità prede
  double B;  // tasso di mortalità prede
  double C;  // tasso di natalità predatori
  double D;  // tasso di mortalità predatori
};

struct State {
  double prey;
  double predator;
  double H;
};

// --------------------------- CLASSE ---------------------------

class Simulation {
  // attributi
  Parameters const par_;
  double const dt_;
  std::size_t const iterations_;
  State state_;
  std::vector<State> evolution_;

  //-----------------------FUNZIONE PRIVATA--------------------------
  static Simulation set_simulation();

 public:
  //-------------------------COSTRUTTORI------------------------------
  Simulation(Parameters p, double prey, double pred, double dt,
             double it);  // principale
  Simulation();           // di default (inizializzzazione)

  //-----------------------FUNZIONI PUBBLICHE-------------------------
  // getter
  Parameters const& parameters() const { return par_; }
  State const& init_state() const { return evolution_.front(); }
  State const& current_state() const { return evolution_.back(); }
  State const& internal_state() const { return state_; }
  double dt() const { return dt_; }
  std::size_t iterations() const { return iterations_; }
  std::vector<State> const& evolution() const { return evolution_; }

  // calcoli
  void evolve();
  void go();

  // file e output
  void save_evolution();
  void save_plot();
};

//--------------------FUNZIONI ESTERNE------------------------
// operatori
bool operator==(Parameters const& a, Parameters const& b);
bool operator==(State const& a, State const& b);

// funzioni libere

double control(const std::string& input);

}  // namespace volterra

#endif
