#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "simulation.hpp"

#include "doctest.h"

TEST_CASE("Testing constructors' throws") {
  SUBCASE("Testing null parameter throws") {
    CHECK_THROWS(volterra::Simulation({42., 0., 42., 42.42}, 4200., 4200.,
                                      0.00042, 4200.));
  }

  SUBCASE("Testing negative parameter throws") {
    CHECK_THROWS(volterra::Simulation({42., 42., -42., 42.42}, 4200., 4200.,
                                      0.00042, 4200.));
  }

  SUBCASE("Testing negative population throws") {
    CHECK_THROWS(volterra::Simulation({42., 42., 42., 42.42}, -4200., 4200.,
                                      0.00042, 4200.));
  }

  SUBCASE("Testing negative time increment throws") {
    CHECK_THROWS(volterra::Simulation({42., 0., 42., 42.42}, 4200., 4200.,
                                      -0.00042, 4200.));
  }

  SUBCASE("Testing negative iterations throws") {
    CHECK_THROWS(volterra::Simulation({42., 0., 42., 42.42}, 4200., 4200.,
                                      0.00042, -4200.));
  }
}

TEST_CASE("Testing object construction") {
  volterra::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.5);

  CHECK(sim.parameters() == volterra::Parameters{800., 1., 1., 1000.});
  CHECK(sim.init_state() == volterra::State{2000., 2000., 0.});
  CHECK(sim.internal_state() == volterra::State{2., 2.5, 0.});
  CHECK(sim.dt() == 0.0001);
  CHECK(sim.iterations() == std::size_t(3));
}

TEST_CASE("Testing computations") {
  volterra::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.);

  SUBCASE("Testing evolve()") {
    sim.evolve();

    CHECK(sim.current_state() == volterra::State{1760., 2200., 0.});
    CHECK(sim.internal_state() == volterra::State{1.76, 2.75, 0.});
  }

  SUBCASE("Testing compute()") {
    sim.compute();

    auto ev = sim.evolution();

    REQUIRE(ev.size() == std::size_t(3));

    std::vector<volterra::State> vec{
        {2000., 2000., 0.}, {1760., 2200., 0.}, {1513.6, 2367.2, 0.}};

    for (std::size_t i{0}; i < ev.size(); ++i) {
      CHECK(ev[i].prey == doctest::Approx(vec[i].prey).epsilon(0.0001));
      CHECK(ev[i].predator == doctest::Approx(vec[i].predator).epsilon(0.0001));
    }
  }
}

TEST_CASE("Testing statistics") {
  volterra::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.);

  sim.compute();
  sim.statistics();

  CHECK(sim.prey_stat().mean == doctest::Approx(1757.867).epsilon(0.01));
  CHECK(sim.pred_stat().mean == doctest::Approx(2189.067).epsilon(0.01));
  CHECK(sim.prey_stat().sigma == doctest::Approx(198.58).epsilon(0.01));
  CHECK(sim.pred_stat().sigma == doctest::Approx(150.11).epsilon(0.01));
  CHECK(sim.prey_stat().maximum == doctest::Approx(2000.).epsilon(0.01));
  CHECK(sim.pred_stat().maximum == doctest::Approx(2367.2).epsilon(0.01));
  CHECK(sim.prey_stat().minimum == doctest::Approx(1513.6).epsilon(0.01));
  CHECK(sim.pred_stat().minimum == doctest::Approx(2000.).epsilon(0.01));
}