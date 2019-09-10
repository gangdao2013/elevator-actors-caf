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
		
	auto passenger = system.spawn<passenger::passenger_actor>(cfg.passenger_name);

	if (!cfg.host.empty() && cfg.port > 0)
		anon_send(passenger, elevator::connect_to_controller_atom::value, cfg.host, cfg.port);
	else
		std::cout << "*** no elevator controller received via config, "
		<< R"(please use "connect <host> <port>" before trying to use the elevator)"
		<< std::endl;

	auto passenger_repl = system.spawn<passenger::passenger_repl_actor>(passenger, "passenger_repl_1");
	anon_send(passenger_repl, elevator::start_atom::value);

	auto self = scoped_actor{ system };
	self->wait_for(passenger, passenger_repl); // will block until passenger and passenger_reply die
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

	auto elevator_repl = system.spawn<elevator::elevator_repl_actor>(elevator, "elevator_repl_1");
	anon_send(elevator_repl, elevator::start_atom::value);

	auto self = scoped_actor{ system };
	self->wait_for(elevator, elevator_repl); // will block until elevator and elevator_repl die
}

void start_controller(actor_system& system, const elevator::config& cfg) 
{
	auto controller = system.spawn<controller::controller_actor>();

	std::cout << ":::try publish at port: " << cfg.port << std::endl;

	auto expected_port = io::publish(controller, cfg.port);
	
	if (!expected_port) {
		std::cerr << ">>> publish failed: "	<< system.render(expected_port.error()) << " <<<" << std::endl;
		return;
	}
	
	std::cout << ":::elevator controller successfully published at port " << *expected_port << std::endl;

	auto controller_repl = system.spawn<controller::controller_repl_actor>(controller, "controller_repl_1");
	anon_send(controller_repl, elevator::start_atom::value);

	auto self = scoped_actor{ system };
	self->wait_for(controller, controller_repl); // will block until controller and controller_repl die

	io::unpublish(controller, 0);
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