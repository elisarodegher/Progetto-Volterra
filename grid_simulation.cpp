#include "grid_simulation.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace wator {

//-------------------------CONSTRUCTOR-------------------------------

GridSimulation::GridSimulation(GridParameters p, unsigned seed)
    : parameters_(p), rng_(seed) {
  if (p.width == 0 || p.height == 0 || p.iterations == 0 ||
      p.fish_density <= 0 || p.sharks_density <= 0 ||
      (p.fish_density + p.sharks_density) > 1 || p.fish_density <= 0 ||
      p.sharks_density <= 0 || p.sharks_initial_energy <= 0 ||
      p.sharks_food_energy <= 0 || p.sharks_breed_energy <= 0 ||
      p.sharks_move_cost <= 0) {
    throw std::invalid_argument("Invalid input.");
  }

  // resizing the grid
  grid_.resize(p.width * p.height);

  std::uniform_real_distribution<double> dist(0.0, 1.0);

  // filling the grid
  for (auto& cell : grid_) {
    double const r = dist(rng_);

    if (r < parameters_.fish_density) {
      cell.state = CellState::Prey;
    } else if (r < parameters_.fish_density + parameters_.sharks_density) {
      cell.state = CellState::Predator;
      cell.energy = parameters_.sharks_initial_energy;
    }
  }

  // updating population
  history_.push_back(get_population());
};

//--------------------------PRIVATE FUNCTIONS-----------------------

// get_population()
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

// index()
std::size_t GridSimulation::index(std::size_t row, std::size_t col) const {
  return row * parameters_.width + col;
}

// current_cell()
Cell& GridSimulation::current_cell(size_t row, size_t column) {
  return grid_[index(row, column)];
};

// up(), down(), left(), right()
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

// neighbours()
std::vector<std::size_t> GridSimulation::neighbours(
    std::size_t row, std::size_t col, CellState chosen_state) const {
  std::vector<std::size_t> result;
  std::array<std::array<std::size_t, 2>, 4> complete_neighb = {
      {{up(row), col}, {row, right(col)}, {down(row), col}, {row, left(col)}}};

  for (auto const& n : complete_neighb) {
    if (grid_[index(n[0], n[1])].state == chosen_state) {
      result.push_back(index(n[0], n[1]));
    }
  }
  return result;
}

// reproduce()
void GridSimulation::reproduce(std::size_t index,
                               std::vector<std::size_t> const& free_neighb,
                               std::size_t max) {
  if (free_neighb.empty()) {
    return;
  }

  std::uniform_int_distribution<std::size_t> dist(0, free_neighb.size() - 1);
  std::size_t child = free_neighb[dist(rng_)];
  // creato l'indice random della cella per il child, preso a caso nel free
  // neighbour

  if (grid_[index].state == CellState::Prey) {
    if (static_cast<std::size_t>(grid_[index].age) < max) {
      return;
    }
    grid_[child].state = CellState::Prey;
    grid_[child].age = 0;
    grid_[index].age = 0;

    // pesce madre si splitta in due pesci figli, uno resta dove era pesce
    // madre, uno va in una cella random, entrambi a età zero

  } else if (grid_[index].state == CellState::Predator) {
    if (static_cast<std::size_t>(grid_[index].energy) < max) {
      return;
    }
    // l'energia del genitore viene divisa a meta' col figlio (come nel modello
    // Wa-Tor)
    int const total_energy = grid_[index].energy;
    grid_[index].energy = total_energy / 2;
    grid_[child].energy = total_energy - grid_[index].energy;
    grid_[child].state = CellState::Predator;
    grid_[child].age = 0;
  }
}

//--------------------------PUBLIC FUNCTIONS-----------------------

// evolve()
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
    if (grid_[j].state == CellState::Empty || already_moved[j])
      continue;  // se la cella è vuota ho ha subito un cambiamento, salta
                 // l'iterazione

    std::size_t const row{j / parameters_.width};
    std::size_t const col{j % parameters_.width};  // FIN QUI UGUALE

    if (grid_[j].state == CellState::Prey) {
      // ---------------- CASO PREDA ----------------
      std::vector<std::size_t> const safe_neighb =
          neighbours(row, col, CellState::Empty);

      if (safe_neighb.empty()) {
        // nessuna cella libera: la preda resta ferma, ma invecchia comunque
        grid_[j].age += 1;
        already_moved[j] = true;
      } else {
        std::uniform_int_distribution<std::size_t> dist(0,
                                                        safe_neighb.size() - 1);
        std::size_t const chosen = safe_neighb[dist(rng_)];

        grid_[chosen] = grid_[j];
        grid_[chosen].age += 1;
        grid_[j] = Cell{};

        already_moved[chosen] = true;
        already_moved[j] = true;

        if (static_cast<std::size_t>(grid_[chosen].age) >=
            static_cast<std::size_t>(parameters_.fish_breed_age)) {
          std::size_t const breed_row{chosen / parameters_.width};
          std::size_t const breed_col{chosen % parameters_.width};
          reproduce(chosen, neighbours(breed_row, breed_col, CellState::Empty),
                    static_cast<std::size_t>(parameters_.fish_breed_age));
        }
      }

    } else {
      // ---------------- CASO PREDATORE ----------------

      std::vector<std::size_t> prey_neighb =
          neighbours(row, col, CellState::Prey);
      std::vector<std::size_t> empty_neighb =
          neighbours(row, col, CellState::Empty);

      std::size_t chosen{j};  // di default resta fermo

      if (!prey_neighb.empty()) {
        std::uniform_int_distribution<std::size_t> dist(0,
                                                        prey_neighb.size() - 1);
        chosen = prey_neighb[dist(rng_)];

        grid_[chosen] = grid_[j];
        grid_[chosen].energy -= parameters_.sharks_move_cost;
        grid_[chosen].energy += parameters_.sharks_food_energy;
        grid_[j] = Cell{};

      } else if (!empty_neighb.empty()) {
        std::uniform_int_distribution<std::size_t> dist(
            0, empty_neighb.size() - 1);
        chosen = empty_neighb[dist(rng_)];

        grid_[chosen] = grid_[j];
        grid_[chosen].energy -= parameters_.sharks_move_cost;
        grid_[j] = Cell{};

      } else {
        // nessun vicino disponibile: resta fermo, ma paga comunque la metà del
        // costo energetico
        grid_[chosen].energy -= parameters_.sharks_move_cost / 2;
      }

      already_moved[chosen] = true;
      already_moved[j] = true;

      if (grid_[chosen].energy <= 0) {
        grid_[chosen] = Cell{};  // muore di fame
      } else if (static_cast<std::size_t>(grid_[chosen].energy) >=
                 static_cast<std::size_t>(parameters_.sharks_breed_energy)) {
        std::size_t const breed_row{chosen / parameters_.width};
        std::size_t const breed_col{chosen % parameters_.width};
        reproduce(chosen, neighbours(breed_row, breed_col, CellState::Empty),
                  static_cast<std::size_t>(parameters_.sharks_breed_energy));
      }
    }
  }
  history_.push_back(get_population());
}

// go()
void GridSimulation::go() {
  while (history_.size() < parameters_.iterations) {
    try {
      this->evolve();
    } catch (const std::runtime_error& error) {
      std::cerr << error.what();
      break;
    }
  }
}

// save_evolution()
void GridSimulation::save_grid_evolution() {
  std::ofstream outfile{"GRID_EVOLUTION.csv"};
  outfile << "step\tfish\tsharks\n";
  std::size_t step{0};
  for (auto const& population : history_) {
    outfile << step << '\t' << population.fish << '\t' << population.sharks
            << '\n';
    ++step;
  }
}

// save_plot()
void GridSimulation::save_grid_plot() {
  std::ofstream data_file{".tmp_grid.csv"};
  std::size_t step{0};

  for (auto const& population : history_) {
    data_file << step << '\t' << population.fish << '\t' << population.sharks
              << '\n';
    ++step;
  }

  data_file.close();

  std::ofstream gp_script{".tmp_grid.gp"};
  gp_script
      << "set terminal svg size 1000,600 font 'Arial,14' background '#99d6e1'\n"
      << "set output 'GRID_PLOT.svg'\n"
      << "set title 'WA-TOR POPULATION'\n"
      << "set xlabel 'STEP'\n"
      << "set ylabel 'INDIVIDUALS'\n"
      << "set xrange [0:" << step - 1 << "]\n"
      << "set autoscale y\n"
      << "set grid\n"
      << "plot '.tmp_grid.csv' using 1:2 with lines lw 2 lc rgb 'green' "
         "title 'FISH', "
      << "'.tmp_grid.csv' using 1:3 with lines lw 2 lc rgb 'blue' title "
         "'SHARKS'\n";

  gp_script.close();

  int const check = std::system("gnuplot .tmp_grid.gp");
  if (check != 0) {
    std::cerr << "gnuplot failed\n";
  }
  std::remove(".tmp_grid.gp");
  std::remove(".tmp_grid.csv");

  std::cout << "\nGrid plot and temporal evolution of the simulation saved.\n";
}

//--------------------EXTERNAL FUNCTIONS------------------------

// operator == Cell
bool operator==(Cell const& a, Cell const& b) {
  return a.state == b.state && a.energy == b.energy && a.age == b.age;
}

// operator == Population
bool operator==(Population const& a, Population const& b) {
  return a.fish == b.fish && a.sharks == b.sharks;
}

// operator == GridParameters
bool operator==(GridParameters const& a, GridParameters const& b) {
  return a.width == b.width && a.height == b.height &&
         a.iterations == b.iterations && a.fish_density == b.fish_density &&
         a.sharks_density == b.sharks_density &&
         a.fish_breed_age == b.fish_breed_age &&
         a.sharks_initial_energy == b.sharks_initial_energy &&
         a.sharks_breed_energy == b.sharks_breed_energy &&
         a.sharks_food_energy == b.sharks_food_energy &&
         a.sharks_move_cost == b.sharks_move_cost;
}

}  // namespace wator
