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
  double a;  // prey birth rate
  double b;  // prey death rate
  double c;  // predator birth rate
  double d;  // predator death rate
};

struct State {
  double x;  // prey
  double y;  // predator
  double H;  // Hamiltonian
};

// --------------------------- CLASS ---------------------------

class Simulation {
  // attributes
  Parameters const parameters_;
  double const timescale_;
  std::size_t const iterations_;
  State state_;
  std::vector<State> evolution_;

  //-----------------------PRIVATE FUNCTION--------------------------
  static Simulation set_simulation();

 public:
  //-------------------------CONSTRUCTORS------------------------------
  Simulation(Parameters p_, double x_, double y_, double dt_,
             double it_);  // main constructor
  Simulation();            // default constructor

  //-----------------------PUBLIC FUNCTIONS-------------------------
  // getters
  Parameters const& parameters() const { return parameters_; }
  State const& initial_state() const { return evolution_.front(); }
  State const& current_state() const { return evolution_.back(); }
  State const& internal_state() const { return state_; }
  double timescale() const { return timescale_; }
  std::size_t iterations() const { return iterations_; }
  std::vector<State> const& evolution() const { return evolution_; }

  // calculations
  void evolve();
  void go();

  // file and output
  void save_evolution();
  void save_plot();
};

//--------------------EXTERNAL FUNCTIONS------------------------
// operators
bool operator==(Parameters const& example1, Parameters const& example2);
bool operator==(State const& example1, State const& example2);

// control function
double control(const std::string&);

}  // namespace volterra

#endif
