#include "grid_simulation.hpp"

#include <algorithm>
#include <array>

namespace wator {

// COSTRUTTORE

GridSimulation::GridSimulation(GridParameters p, unsigned seed)
    : parameters_(p), rng_(seed) {
  if (p.width == 0 || p.height == 0 || p.iterations <= 0 ||
      p.fish_density <= 0 || p.sharks_density <= 0 ||
      (p.fish_density + p.sharks_density) > 1 || p.fish_density <= 0 ||
      p.sharks_density <= 0 || p.sharks_initial_energy <= 0 ||
      p.sharks_food_energy <= 0 || p.sharks_breed_energy <= 0 ||
      p.sharks_move_cost <= 0) {
    throw std::invalid_argument("Invalid input.");
  }

  grid_.resize(p.width * p.height);

  std::uniform_real_distribution<double> dist(0.0, 1.0);

  for (auto& cell : grid_) {
    double const r = dist(rng_);

    if (r < parameters_.fish_density) {
      cell.state = CellState::Prey;
    } else if (r < parameters_.fish_density + parameters_.sharks_density) {
      cell.state = CellState::Predator;
      cell.energy = parameters_.sharks_initial_energy;
    }
  }
  history_.push_back(get_population());
};

// ---------------METODI DELLA CLASSE------------------

// ---------------PRIVATI------------------------------

Population GridSimulation::get_population() const {
  Population population;

  for (Cell const& cell : grid_) {
    if (cell.state == CellState::Prey)
      ++population.fish;
    else if (cell.state == CellState::Predator)
      ++population.sharks;
  }

  return population;
}

std::size_t GridSimulation::index(std::size_t row, std::size_t col) const {
  return row * parameters_.width + col;
}

Cell& GridSimulation::current_cell(size_t row, size_t column) {
  return grid_[index(row, column)];
};

std::size_t GridSimulation::up(std::size_t row) const {
  return (row + parameters_.height - 1) % parameters_.height;
}

std::size_t GridSimulation::down(std::size_t row) const {
  return (row + 1) % parameters_.height;
}

std::size_t GridSimulation::left(std::size_t col) const {
  return (col + parameters_.width - 1) % parameters_.width;
}

std::size_t GridSimulation::right(std::size_t col) const {
  return (col + 1) % parameters_.width;
}

// PUBBLICI serve fare evolve e go

void GridSimulation::evolve() {
  std::vector<std::size_t>
      occupied;  // contiene gli indici delle caselle occupate ex (1,3,4)
  for (std::size_t i = 0; i < grid_.size(); ++i) {
    if (grid_[i].state != CellState::Empty) occupied.push_back(i);
  }
  std::shuffle(occupied.begin(), occupied.end(),
               rng_);  // per questo serve algorithm
  std::vector<bool> already_moved(grid_.size(), false);

  for (std::size_t j : occupied) {
    // forse controllo di cellstate empty è ridondante??
    if (grid_[j].state == CellState::Empty || already_moved[j])
      continue;  // se la cella è vuota ho ha subito un cambiamento, salta
                 // l'iterazione

    std::size_t row{j / parameters_.width};
    std::size_t col{j % parameters_.width};
    std::array<std::array<std::size_t, 2>, 4> neighbours = {{{up(row), col},
                                                             {row, right(col)},
                                                             {down(row), col},
                                                             {row, left(col)}}};

    if (grid_[j].state ==
        CellState::Prey) {  // ------------------------------CASO PREDA
      std::vector<std::size_t> danger_neighb;
      std::vector<std::size_t> safe_neighb;

      for (std::size_t i = 0; i < neighbours.size(); ++i) {
        std::size_t neighbour_row = neighbours[i][0];
        std::size_t neighbour_col = neighbours[i][1];

        if (current_cell(neighbour_row, neighbour_col).state ==
            CellState::Empty) {
          safe_neighb.push_back(index(neighbour_row, neighbour_col));
        }
      }

      /*se il vicinato è safe, ovvero non contiene squali , si sceglie una
       * cella random tra quelle*/
      std::uniform_int_distribution<std::size_t> dist(0,
                                                      safe_neighb.size() - 1);

      std::size_t chosen = safe_neighb[dist(rng_)];

      grid_[chosen] = grid_[j];
      grid_[chosen].age += 1;
      grid_[j] = Cell{};

      already_moved[chosen] = true;

      // ANCORA DA GESTIRE RIPRODUZIONE PREDE

    } else {  // -----------------------------------CASO PREDATORE
      std::vector<std::size_t> prey_neighb;
      std::vector<std::size_t> empty_neighb;

      for (std::size_t i = 0; i < neighbours.size(); ++i) {
        std::size_t neighbour_row = neighbours[i][0];
        std::size_t neighbour_col = neighbours[i][1];

        if (current_cell(neighbour_row, neighbour_col).state ==
            CellState::Prey) {
          prey_neighb.push_back(index(neighbour_row, neighbour_col));
        } else if (current_cell(neighbour_row, neighbour_col).state ==
                   CellState::Empty) {
          empty_neighb.push_back(index(neighbour_row, neighbour_col));
        }

        /*se il vicinato di prede è vuoto, si sposta nel vicinato vuoto,
         * altrimenti va a mangare la preda. in entrambi casi c'è un move
         * cost, solo se mangia la preda acquista energia*/

        if (prey_neighb.empty()) {
          std::uniform_int_distribution<std::size_t> dist(
              0, empty_neighb.size() - 1);

          std::size_t chosen = empty_neighb[dist(rng_)];

          grid_[chosen] = grid_[j];
          grid_[chosen].energy -= parameters_.sharks_move_cost;
          grid_[j] = Cell{};

          already_moved[chosen] = true;
        } else {
          std::uniform_int_distribution<std::size_t> dist(
              0, prey_neighb.size() - 1);

          std::size_t chosen = prey_neighb[dist(rng_)];

          grid_[chosen] = grid_[j];

          // calcolo della spesa / guadagno di energia
          grid_[chosen].energy -= parameters_.sharks_move_cost;
          grid_[chosen].energy += parameters_.sharks_food_energy;

          // la casella chosen ha cambiato stato

          already_moved[chosen] = true;

          // in j devo impostare che la cell è vuota
          grid_[j] = Cell{};  // struct cell con i valori di default quindi
                              // azzera tutto
        }
      }

      // ancora da gestire la riproduzione dello squalo e la sua eventuale morte
    }

    history_.push_back(get_population());
  }
}

// EXTERNAL FUNCTIONS
bool operator==(Cell const& a, Cell const& b) {
  return a.state == b.state && a.energy == b.energy && a.age == b.age;
}

bool operator==(Population const& a, Population const& b) {
  return a.fish == b.fish && a.sharks == b.sharks;
}

}  // namespace wator
