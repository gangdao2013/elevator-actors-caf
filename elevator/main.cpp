#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"

#include "elevator/controller_actor.hpp"
#include "elevator/passenger_actor.hpp"
#include "elevator/elevator_actor.hpp"

using namespace caf;

//
// main.cpp
//
/*
Harness application to demo the elevator actor system.

It operates in one of three modes, and you will need to have at least one of each mode running in its own command window:

Controller mode: 
----------------
Runs as the main controller/supervisor for the system. An embedded controller actor and linked controller REPL actor will start.
It doesn't (yet) monitor/respawn actors, but planned for the future. 

Usage:

> elevator.exe -C --port <port number, default 10000> 
...will start a controller on localhost on default port 10000 (you can change this in elevator/elevator.hpp). Run one controller per session.

Elevator mode: 
--------------
Runs as an elevator. An embedded elevator actor and linked elevator REPL will start.

Usage:
> elevator.exec -E --host <host-name> --port <port number, default 10000>
...will start an elevator on localhost, connecting to your controller on host/port. If not specified host/port will be localhost/1000.

You can run multiple elevators per session, in separate command windows. Each elevator has default capacity of 2 passengers, 
change this in elevator/elevator.hpp. Future versions will have capacity be a command line option.

Passenger mode:
--------------
Runs as a passenger. An embedded passenger actor and linked passenger REPL will start.

Usage:
> elevator.exec -P --name <passenger-name> --host <host-name> --port <port number, default 10000>
...will start an passenger on localhost, connecting to your controller on host/port. If not specified host/port will be localhost/1000.

You can run multiple passengers per session, in separate command windows. To have have passengers call for lifts, in the REPL just type 'c <floor_to>', 
e.g. 'c 6' - this will call for an elevator from the passenger's current floor to floor 6. 

Have an experiment with multiple passengers (in multiple command windows) calling for elevators (running in their own command windows.)
Each actor prints a detailed log of their events & actions.


*/


// Start passenger mode
void start_passenger(actor_system& system, const elevator::config& cfg)
{
		
	auto passenger = system.spawn<passenger::passenger_actor>(cfg.passenger_name);

	if (!cfg.host.empty() && cfg.port > 0)
		anon_send(passenger, elevator::connect_to_controller_atom::value, cfg.host, cfg.port);
	else
		std::cout << "*** no elevator controller received via config, "
		<< R"(please use "connect <host> <port>" before trying to use the elevator)"
		<< std::endl;

	// create and start the repl
	auto passenger_repl = system.spawn<passenger::passenger_repl_actor>(passenger, "passenger_repl_1");
	anon_send(passenger_repl, elevator::start_atom::value);

	auto self = scoped_actor{ system };
	self->wait_for(passenger, passenger_repl); // will block until passenger and passenger_reply die
}

// Start elevator mode
void start_elevator(actor_system& system, const elevator::config& cfg)
{
	auto elevator = system.spawn<elevator::elevator_actor>(1);

	if (!cfg.host.empty() && cfg.port > 0)
		anon_send(elevator, elevator::connect_to_controller_atom::value, cfg.host, cfg.port);
	else
		std::cout << "*** no elevator controller received via config, "
		<< R"(please use "connect <host> <port>" before trying to use the elevator)"
		<< std::endl;

	// create and start the repl
	auto elevator_repl = system.spawn<elevator::elevator_repl_actor>(elevator, "elevator_repl_1");
	anon_send(elevator_repl, elevator::start_atom::value);

	auto self = scoped_actor{ system };
	self->wait_for(elevator, elevator_repl); // will block until elevator and elevator_repl die
}

// Start controller mode
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

	// create and start the repl
	auto controller_repl = system.spawn<controller::controller_repl_actor>(controller, "controller_repl_1");
	anon_send(controller_repl, elevator::start_atom::value);

	auto self = scoped_actor{ system };
	self->wait_for(controller, controller_repl); // will block until controller and controller_repl die

	io::unpublish(controller, 0); // need this last step so the system doesn't hang
}

typedef void(*starter_f)(actor_system& system, const elevator::config& cfg);

// caf_main is the psuedo main function for the system - eventually called by the CAF_MAIN macro below

void caf_main(actor_system& system, const elevator::config& cfg) {
	
	starter_f f = nullptr;

	// which mode?
	if (cfg.controller_mode)
		f = start_controller;
	else if (cfg.passenger_mode)
		f = start_passenger;
	else if (cfg.elevator_mode)
		f = start_elevator;

	// let's go!
	if (f != nullptr)
		f(system, cfg);

}

// CAF main function, including network-enabled communications between remote actors:
CAF_MAIN(io::middleman)