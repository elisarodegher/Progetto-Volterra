#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include ".doctest.h"
#include "simulation.hpp"

TEST_CASE("Testing constructors' throws") {
  SUBCASE("Testing null parameter throws") {
    CHECK_THROWS(lv::Simulation({42., 0., 42., 42.42}, 4200., 4200., 0.00042, 4200.));
  }

  SUBCASE("Testing negative parameter throws") {
    CHECK_THROWS(lv::Simulation({42., 42., -42., 42.42}, 4200., 4200., 0.00042, 4200.));
  }

  SUBCASE("Testing negative population throws") {
    CHECK_THROWS(lv::Simulation({42., 42., 42., 42.42}, -4200., 4200., 0.00042, 4200.));
  }

  SUBCASE("Testing negative time increment throws") {
    CHECK_THROWS(lv::Simulation({42., 0., 42., 42.42}, 4200., 4200., -0.00042, 4200.));
  }

  SUBCASE("Testing negative iterations throws") {
    CHECK_THROWS(lv::Simulation({42., 0., 42., 42.42}, 4200., 4200., 0.00042, -4200.));
  }

  SUBCASE("Testing inexistent initialization file throws") { CHECK_THROWS(lv::Simulation("prank.txt")); }

  SUBCASE("Testing incomplete initialization file throws") {
    CHECK_THROWS(lv::Simulation(".bad-init-files/incomplete_init1.txt"));
    CHECK_THROWS(lv::Simulation(".bad-init-files/incomplete_init2.txt"));
  }
}

TEST_CASE("Testing object construction") {
  lv::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.5);

  CHECK(sim.parameters() == lv::Parameters{800., 1., 1., 1000.});
  CHECK(sim.init_state() == lv::State{2000., 2000., 0.});
  CHECK(sim.internal_state() == lv::State{2., 2.5, 0.});
  CHECK(sim.dt() == 0.0001);
  CHECK(sim.iterations() == std::size_t(3));
}

TEST_CASE("Testing computations") {
  lv::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.);

  SUBCASE("Testing evolve()") {
    sim.evolve();

    CHECK(sim.current_state() == lv::State{1760., 2200., 0.});
    CHECK(sim.internal_state() == lv::State{1.76, 2.75, 0.});
  }

  SUBCASE("Testing compute()") {
    sim.compute();

    auto ev = sim.evolution();

    REQUIRE(ev.size() == std::size_t(3));

    std::vector<lv::State> vec{{2000., 2000., 0.}, {1760., 2200., 0.}, {1513.6, 2367.2, 0.}};

    for (std::size_t i{0}; i < ev.size(); ++i) {
      CHECK(ev[i].prey == doctest::Approx(vec[i].prey).epsilon(0.0001));
      CHECK(ev[i].predator == doctest::Approx(vec[i].predator).epsilon(0.0001));
    }
  }
}

TEST_CASE("Testing statistics") {
  lv::Simulation sim({800., 1., 1., 1000.}, 2000., 2000., 0.0001, 3.);

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
