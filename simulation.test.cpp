#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "simulation.hpp"

#include "doctest.h"

TEST_CASE("Testing constructors' throws") {
  SUBCASE("Testing null a throws") {
    CHECK_THROWS(volterra::Simulation({0., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing null b parameter throws") {
    CHECK_THROWS(volterra::Simulation({49., 0., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing null c parameter throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 0., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }
  SUBCASE("Testing null d parameter throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 0.}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative a parameter throws") {
    CHECK_THROWS(volterra::Simulation({-49., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative b parameter throws") {
    CHECK_THROWS(volterra::Simulation({49., -49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative c parameter throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., -49., 49.4}, 4900., 4900.,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative d parameter throws") {
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

  SUBCASE("Testing null prey population throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 0.0, 4900.,
                                      0.00049, 4900.));
  }
  SUBCASE("Testing null predator population throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 0.0,
                                      0.00049, 4900.));
  }

  SUBCASE("Testing negative time increment throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900.,
                                      -0.00049, 4900.));
  }

  SUBCASE("Testing null dt throws") {
    CHECK_THROWS(
        volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900., 0.0, 4900.));
  }

  SUBCASE("Testing negative iterations throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, -4900.));
  }
  SUBCASE("Testing null iterations throws") {
    CHECK_THROWS(volterra::Simulation({49., 49., 49., 49.4}, 4900., 4900.,
                                      0.00049, 0.0));
  }
}


TEST_CASE("Testing object construction") {
  volterra::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.5);

  CHECK(sim.parameters() == volterra::Parameters{800., 1., 1., 1000.});
  CHECK(sim.initial_state() == volterra::State{2000., 2000., 0.});
  CHECK(sim.internal_state() == volterra::State{2., 2.5, 0.});
  CHECK(sim.timescale() == 0.0001);
  CHECK(sim.iterations() == std::size_t(3));
}

TEST_CASE("Testing sim") {
  volterra::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.);

  SUBCASE("Testing evolve()") {
    sim.evolve();

    CHECK(sim.current_state() == volterra::State{1760., 2200., 0.});
    CHECK(sim.internal_state() == volterra::State{1.76, 2.75, 0.});
  }

  SUBCASE("Testing go()") {
    sim.go();

    auto ev = sim.evolution();

    REQUIRE(ev.size() == std::size_t(3));

    std::vector<volterra::State> vec{
        {2000., 2000., 0.}, {1760., 2200., 0.}, {1513.6, 2367.2, 0.}};

    for (std::size_t i{0}; i < ev.size(); ++i) {
      CHECK(ev[i].x == doctest::Approx(vec[i].x).epsilon(0.0001));
      CHECK(ev[i].y == doctest::Approx(vec[i].y).epsilon(0.0001));
    }
  }
}
