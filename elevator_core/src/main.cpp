#include <iostream>
#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/controller_actor.hpp"
#include "elevator/passenger_actor.hpp"
#include "elevator/elevator_actor.hpp"

using namespace caf;

void start_passenger(actor_system& system, const elevator::config& cfg)
{
	auto passenger = system.spawn<passenger::passenger_actor>("Fred");

	if (!cfg.host.empty() && cfg.port > 0)
		anon_send(passenger, elevator::connect_to_controller_atom::value, cfg.host, cfg.port);
	else
		std::cout << "*** no elevator controller received via config, "
		<< R"(please use "connect <host> <port>" before trying to use the elevator)"
		<< std::endl;

	passenger::passenger_repl repl(system, passenger);
	repl.start_repl(); // will run until quit entered

	auto self = scoped_actor{ system };
	self->wait_for(passenger); // will block until passenger dies
}

void start_elevator(actor_system& system, const elevator::config& cfg)
{
	auto elevator = system.spawn<elevator::elevator_actor>(1);

	if (!cfg.host.empty() && cfg.port > 0)
		anon_send(elevator, elevator::connect_to_controller_atom::value, cfg.host, cfg.port);
	else
		std::cout << "*** no elevator controller received via config, "
		<< R"(please use "connect <host> <port>" before trying to use the elevator)"
		<< std::endl;

	elevator::elevator_repl repl(system, elevator);
	repl.start_repl(); // will run until quit entered

	auto self = scoped_actor{ system };
	self->wait_for(elevator); // will block until passenger dies
}

void start_controller(actor_system& system, const elevator::config& cfg) {
	auto controller = system.spawn(controller::controller_actor);

	std::cout << "*** try publish at port " << cfg.port << std::endl;
	auto expected_port = io::publish(controller, cfg.port);
	if (!expected_port) {
		std::cerr << "*** publish failed: "
			<< system.render(expected_port.error()) << std::endl;
		return;
	}
	std::cout << "*** elevator controller successfully published at port " << *expected_port << std::endl
		<< "*** press [enter] to quit" << std::endl;
	std::string dummy;
	std::getline(std::cin, dummy);
	std::cout << "... cya" << std::endl;
	anon_send_exit(controller, exit_reason::user_shutdown);
}

typedef void(*starter_f)(actor_system& system, const elevator::config& cfg);

void caf_main(actor_system& system, const elevator::config& cfg) {
	
	starter_f f = nullptr;
	if (cfg.controller_mode)
		f = start_controller;
	else if (cfg.passenger_mode)
		f = start_passenger;
	else if (cfg.elevator_mode)
		f = start_elevator;

	if (f != nullptr)
		f(system, cfg);

}

CAF_MAIN(io::middleman)