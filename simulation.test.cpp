#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "simulation.hpp"

#include "doctest.h"

TEST_CASE("Testing constructors' throws") {
  SUBCASE("Testing null parameter throws") {
    CHECK_THROWS(volterra::Simulation({49., 0., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative a throws") {
    CHECK_THROWS(volterra::Simulation({-49., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative b throws") {
    CHECK_THROWS(volterra::Simulation({49., -49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative c throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., -49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative d throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., -49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative prey population throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, -4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative predator population throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., -4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative time increment throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900.,
                                      -0.00049, 4900.));
  }

  SUBCASE("Testing negative iterations throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, -4900.));
  }
}

TEST_CASE("Testing object construction") {
  volterra::Simulation sim({500., 2., 4., 1000.}, 1000., 250., 0.0002, 4.8);

  CHECK(sim.parameters() == volterra::Parameters{500., 2., 4., 1000.});
  CHECK(sim.initial_state() == volterra::State{1000., 250., 0.});
  CHECK(sim.internal_state() == volterra::State{4., 1., 0.});
  CHECK(sim.timescale() == 0.0002);
  CHECK(sim.iterations() == std::size_t(4));
}

TEST_CASE("Testing sim") {
  volterra::Simulation sim({500., 2., 4., 1000.}, 1000., 250., 0.0002, 4.8);

  SUBCASE("Testing evolve()") {
  sim.evolve();

  CHECK(sim.current_state() == volterra::State{1000., 400., 0.});
  CHECK(sim.internal_state() == volterra::State{4., 1.6, 0.});
}

  SUBCASE("Testing go()") {
  sim.go();

  auto ev = sim.evolution();
  REQUIRE(ev.size() == std::size_t(4));

  std::vector<volterra::State> vec{
      {1000., 250., 0.},
      {1000., 400., 0.},
      {1000., 640., 0.},
      {1000., 1024., 0.}
  };

  for (std::size_t i{0}; i < ev.size(); ++i) {
    CHECK(ev[i].x == doctest::Approx(vec[i].x).epsilon(0.0001));
    CHECK(ev[i].y == doctest::Approx(vec[i].y).epsilon(0.0001));
  }
}
}
