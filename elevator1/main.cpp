
#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "controller.hpp"
#include "elevator.hpp"
#include "passenger.hpp"

using namespace caf;

void caf_main(actor_system& system, const elevator::config& cfg) {
	//auto f = cfg.controller_mode ? controller::start_controller : passenger::start_passenger;
	auto f = passenger::start_passenger;
	f(system, cfg);
}

CAF_MAIN(io::middleman)