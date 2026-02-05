#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "simulation.hpp"

#include "doctest.h"

TEST_CASE("Testing constructors") {
  SUBCASE(" a=0 ") {
    CHECK_THROWS(volterra::Simulation({0., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE(" b=0 ") {
    CHECK_THROWS(volterra::Simulation({49., 0., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE(" c=0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 0., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }
  SUBCASE(" d=0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 0.}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE(" a<0 ") {
    CHECK_THROWS(volterra::Simulation({-49., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE(" b<0 ") {
    CHECK_THROWS(volterra::Simulation({49., -49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE(" c<0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., -49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE(" d<0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., -49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE(" x<0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, -4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE(" y<0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., -4900.,
                                      0.00049, 4900.));
  }

  SUBCASE(" x=0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 0.0, 4900.,
                                      0.00049, 4900.));
  }
  SUBCASE(" y=0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 0.0,
                                      0.00049, 4900.));
  }

  SUBCASE(" dt<0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900.,
                                      -0.00049, 4900.));
  }

  SUBCASE(" dt=0 ") {
    CHECK_THROWS(
        volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900., 0.0, 4900.));
  }

  SUBCASE(" it<0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, -4900.));
  }
  SUBCASE(" it=0 ") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 0.0));
  }
}

TEST_CASE(" Testing initialization ") {
  volterra::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.5);

  CHECK(sim.parameters() == volterra::Parameters{800., 1., 1., 1000.});
  CHECK(sim.initial_state() == volterra::State{2000., 2000., 0.});
  CHECK(sim.internal_state() == volterra::State{2., 2.5, 0.});
  CHECK(sim.timescale() == 0.0001);
  CHECK(sim.iterations() == std::size_t(3));
}

TEST_CASE("Testing methods") {
  volterra::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.);

  SUBCASE("Testing evolve()") {
    sim.evolve();

    CHECK(sim.current_state() == volterra::State{1760., 2200., 0.});
    CHECK(sim.internal_state() == volterra::State{1.76, 2.75, 0.});
  }

  SUBCASE("Testing go()") {
    sim.go();

    auto evolution = sim.evolution();

    REQUIRE(evolution.size() == std::size_t(3));

    std::vector<volterra::State> expected{
        {2000., 2000., 0.}, {1760., 2200., 0.}, {1513.6, 2367.2, 0.}};

    for (std::size_t i{0}; i < expected.size(); ++i) {
      CHECK(evolution[i].x == doctest::Approx(expected[i].x).epsilon(1e-10));
      CHECK(evolution[i].y == doctest::Approx(expected[i].y).epsilon(1e-10));
    }
  }
}
