#ifndef LV_PROJECT_HPP
#define LV_PROJECT_HPP

#include <cmath>
#include <iostream>
#include <vector>

namespace volterra {

struct Parameters {  // contiene i parametri (coefficienti) della simulazione.
  double A;          // prey birth rate
  double B;          // prey death rate
  double C;          // predator birth rate
  double D;          // predator death rate
};

struct State {  // Questa struct contiene le variabili dell'eq. diff: numero di
                // prede, predatori e integrale primo
  double prey;
  double predator;
  double H;
};

struct Statistics {  // struct dei risultati, ovvero delle statistiche.
  double mean;
  double sigma;
  double maximum;
  double minimum;
};

class Simulation {
  Parameters const par_;          // parametri della simulazione
  double const dt_;               // passo dt temporale
  std::size_t const iterations_;  // numero di interazioni, size_t è solo un
                                  // altro nome per unsigned long
  State state_;                   // stato istantaneo della simulazione
  std::vector<State> evolution_;  // vettore con tutti gli stati in fila
  Statistics prey_stat_;          // statistiche della preda
  Statistics pred_stat_;          // statistiche del predatore

  explicit Simulation(Parameters par, double prey, double pred, double dt,
                      std::size_t it)  // Costruttore principale di Simulation
      : par_(par), dt_(dt), iterations_(it) {
    // Calcolo di H iniziale:
    state_.H = (par_.C * prey) + (par_.B * pred) -
               ((par_.D * std::log(prey)) + (par_.A * std::log(pred)));

    evolution_.push_back(
        State{prey, pred, state_.H});  // vettore che contiene gli stati del
                                       // sistema messi in ordine cronologico

    // Cambio di coordinate (H è invariante al cambio di sdr)
    state_.prey = prey * par_.C / par_.D;
    state_.predator = pred * par_.B / par_.A;
  }
  // sono le funzioni che preparano la simulazione
  static Simulation set_simulation();
  static Simulation set_simulation(std::string const& file_name);

  // funzioni per controllare la validità dei parametri
  static Parameters const& is_good(Parameters const& p);
  static double is_good(double val);
  static double dt_good(double t);
  static std::size_t it_good(double val);

 public:
  // Costruttore pubblico
  Simulation(Parameters p, double prey, double pred, double t, double n)
      : Simulation(is_good(p), is_good(prey), is_good(pred), dt_good(t),
                   it_good(n)) {}

  // Creazione di una simulazione leggendo i dati dall'utente
  Simulation() : Simulation(set_simulation()) {}

  // Creazione di una simulazione leggendo un file
  Simulation(std::string const& name) : Simulation(set_simulation(name)) {}

  // Funzioni di lettura di parametri
  Parameters const& parameters() const { return par_; }

  State const& init_state() const { return evolution_.front(); }

  State const& current_state() const { return evolution_.back(); }

  State const& internal_state() const { return state_; }

  double dt() const { return dt_; }

  std::size_t iterations() const { return iterations_; }

  std::vector<State> const& evolution() const { return evolution_; }

  Statistics const& prey_stat() { return prey_stat_; }

  Statistics const& pred_stat() { return pred_stat_; }

  // Funzioni di calcolo

  void evolve();  // fa evolvere la simulazione

  void compute();  // fa i calcoli

  void statistics();  // ritorna le statistiche

  // funzioni di presentazione dei risultati

  void save_evolution(std::string const& name) const;

  void save_trajectory(std::string const& file_name) const;

  void save_statistics(std::string const& name);
};

// Definizione di operatori

bool operator==(Parameters const& a, Parameters const& b);

bool operator==(State const& a, State const& b);

// Funzioni fuori dalla classe (libere)

bool comp_prey(State const& a, State const& b);

bool comp_pred(State const& a, State const& b);

bool read(std::ifstream& in, std::string& str, char const delimiter);

}  // namespace volterra
#endif
