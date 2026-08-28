#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "grid_simulation.hpp"

#include "doctest.h"

namespace {

wator::GridParameters valid_parameters() {
  return wator::GridParameters{
      10,  10,  20,  // width, height, iterations
      0.3, 0.1,      // fish_density, sharks_density
      15,            // fish_breed_age
      20,  30,  20,
      3  // sharks_initial_energy, breed_energy, food_energy, move_cost
  };
}

}  // namespace

TEST_CASE("Testing constructor validation") {
  auto p = valid_parameters();

  SUBCASE("width = 0") {
    auto bad = p;
    bad.width = 0;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("height = 0") {
    auto bad = p;
    bad.height = 0;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("iterations = 0") {
    auto bad = p;
    bad.iterations = 0;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("fish_density <= 0") {
    auto bad = p;
    bad.fish_density = 0;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("sharks_density <= 0") {
    auto bad = p;
    bad.sharks_density = 0;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("fish_density + sharks_density > 1") {
    auto bad = p;
    bad.fish_density = 0.7;
    bad.sharks_density = 0.5;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("sharks_initial_energy <= 0") {
    auto bad = p;
    bad.sharks_initial_energy = 0;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("sharks_food_energy <= 0") {
    auto bad = p;
    bad.sharks_food_energy = 0;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("sharks_breed_energy <= 0") {
    auto bad = p;
    bad.sharks_breed_energy = 0;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("sharks_move_cost <= 0") {
    auto bad = p;
    bad.sharks_move_cost = 0;
    CHECK_THROWS(wator::GridSimulation(bad, 1));
  }

  SUBCASE("valid parameters do not throw") {
    CHECK_NOTHROW(wator::GridSimulation(p, 1));
  }
}

TEST_CASE("Testing initialization") {
  auto p = valid_parameters();
  wator::GridSimulation sim(p, 42);

  CHECK(sim.parameters() == p);
  CHECK(sim.width() == p.width);
  CHECK(sim.height() == p.height);
  CHECK(sim.iterations() == static_cast<std::size_t>(p.iterations));

  // checking if history starts with a singular element
  REQUIRE(sim.history().size() == std::size_t(1));

  auto const& initial = sim.history().front();
  std::size_t const grid_size = p.width * p.height;

  CHECK(initial.fish <= grid_size);
  CHECK(initial.sharks <= grid_size);
  CHECK(initial.fish + initial.sharks <= grid_size);
}

TEST_CASE("Testing evolve()") {
  auto p = valid_parameters();
  wator::GridSimulation sim(p, 42);
  std::size_t const grid_size = p.width * p.height;

  SUBCASE("population never exceeds grid size") {
    for (int i = 0; i < 50; ++i) {
      sim.evolve();
      auto const& last = sim.history().back();
      CHECK(last.fish <= grid_size);
      CHECK(last.sharks <= grid_size);
      CHECK(last.fish + last.sharks <= grid_size);
    }
  }

  SUBCASE("a single evolve() adds exactly one history entry") {
    std::size_t const before = sim.history().size();
    sim.evolve();
    CHECK(sim.history().size() == before + 1);
  }

  SUBCASE("repeated evolve() does not throw or crash") {
    CHECK_NOTHROW({
      for (int i = 0; i < 200; ++i) sim.evolve();
    });
  }
}

TEST_CASE("Testing go()") {
  auto p = valid_parameters();
  p.iterations = 15;
  wator::GridSimulation sim(p, 7);
  sim.go();
  CHECK(sim.history().size() == static_cast<std::size_t>(p.iterations));
}

// testing determinism
TEST_CASE("same seed gives same population history") {
  auto p = valid_parameters();
  p.iterations = 10;

  wator::GridSimulation sim1(p, 99);
  wator::GridSimulation sim2(p, 99);

  sim1.go();
  sim2.go();

  REQUIRE(sim1.history().size() == sim2.history().size());
  for (std::size_t i = 0; i < sim1.history().size(); ++i) {
    CHECK(sim1.history()[i] == sim2.history()[i]);
  }
}
