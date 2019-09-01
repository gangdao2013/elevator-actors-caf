#include <iostream>
#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "controller.hpp"
#include "elevator.hpp"
#include "passenger_actor.hpp"
//#include "passenger_repl.hpp"

using namespace caf;
using namespace passenger;


void start_passenger(actor_system& system, const elevator::config& cfg);


void start_passenger(actor_system& system, const elevator::config& cfg)
{
	passenger_repl::usage();

	auto passenger = system.spawn<passenger_actor>();

	if (!cfg.host.empty() && cfg.port > 0)
		anon_send(passenger, elevator::connect_to_controller_atom::value, cfg.host, cfg.port);
	else
		std::cout << "*** no elevator controller received via config, "
		<< R"(please use "connect <host> <port>" before trying to use the elevator)"
		<< std::endl;


	auto self = scoped_actor{ system };
	self->wait_for(passenger);
}


void caf_main(actor_system& system, const elevator::config& cfg) {
	auto f = cfg.controller_mode ? controller::start_controller : start_passenger;
	//auto f = start_passenger;
	f(system, cfg);
}

CAF_MAIN(io::middleman)