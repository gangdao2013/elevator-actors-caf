#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "string_util.hpp"

#include "elevator.hpp"
#include "controller.hpp"
#include "passenger.hpp"

using namespace caf;
using namespace std;
using namespace elevator;


//using down_atom = atom_constant<atom("go_down")>;
//

namespace passenger
{

	behavior passenger_actor::make_behavior()
	{
		return {
			[=](connect_to_controller_atom, const std::string& host, uint16_t port) {
				aout(this) << "\npassenger: connect_atom received" << endl;
				//connecting(this, host, port);
			},
				[=](elevator::connect_to_controller_atom) { 
				aout(this) << "\nconnecting" << endl; 
				//get_instr(); 
			},
			[=](elevator::call_atom, int to_floor) 
			{ 
				aout(this) << "\ncalling" << endl;
				//get_instr();
			},
			[=](elevator::quit_atom) 
			{
				anon_send_exit(this, exit_reason::user_shutdown); 
			},
			[=](destination_arrived_atom)
			{
				aout(this) << "\npassenger: destination_arrived_atom received" << endl;
				//self->become(waiting_for_instruction(self));
				//waiting_for_instruction(self);
			}
		};
	}





	// State transition for the passenger for connecting to the elevator controller, making calls, in transit, etc:
	//
	//                    +-------------+
	//                    |    init     |
	//                    +-------------+
	//                           |
	//                           V
	//                    +-------------+
	//                    | unconnected |<------------------+
	//                    +-------------+                   |
	//                           |                          |
	//                           | {connect Host Port}      |
	//                           |                          |
	//                           V                          |
	//                    +-------------+  {error}          |
	//    +-------------->| connecting  |-------------------+
	//    |               +-------------+                   |
	//    |                      |                          |
	//    |                      | {ok, connected}          |
	//    |{connect Host Port}   |                          |
	//    |                      V                          |
	//    |               +-------------+ {DOWN controller} |
	//    +---------------|   waiting   |-------------------+
	//                    | for instruction|<------+       
	//	                  +-------------+          |
	//	                        |                  |
	//       {lift arrives}     |                  | {arrived at destination/disembark}
	//                          V                  |
	//                    +-------------+          |
	//                    | in lift/    |          |
	//                    | in transit  |----------+
	//                    +-------------+



	struct state {
		strong_actor_ptr current_controller;
		strong_actor_ptr repl;
		//const actor& repl;
		int current_floor = 0;
		int called_floor = 0;
	};

	// starting point of our passenger FSM
	void passenger_actor::initialising()
	{

		// transition to `unconnected` on elevator controller failure/shutdown
		// set the handler if we lose connection to elevator controller 
		set_down_handler([=](const down_msg& dm) {
			if (dm.source == controller) {
				aout(this) << "\npassenger: lost connection to elevator controller, please reconnect or quit" << endl;
				controller = nullptr;
				//become(unconnected(self));
				// transition to unconnected
			}
			});
	}

	void Initialising::entry()
	{
	}



	// prototype definition for unconnected state
	behavior unconnected(stateful_actor<state>* self);

	// prototype definition for transition to `connecting` with controller on host and port
	void connecting(stateful_actor<state>* self, const std::string& host, uint16_t port);

	// waiting for instructions from user
	behavior waiting_for_instruction(stateful_actor<state>* self);

	// prototype definition for transition to `waiting for elevator` 
	behavior waiting_for_elevator(stateful_actor<state>* self);

	// prototype definition for transition to `in in lift\in transit`
	behavior in_transit(stateful_actor<state>* self);

	//void passenger_repl(actor_system& system, const config& cfg);
	behavior passenger_repl(event_based_actor* self);




	// unconnected from elevator controller, waiting for message to 'connect'
	behavior unconnected(stateful_actor<state>* self)
	{
		aout(self) << "\npassenger: entering unconnected" << endl;

		auto repl = self->spawn(passenger_repl);
		self->send(repl, elevator::get_instructions_atom::value, self->state.current_floor);

		return {
		  [=](connect_to_controller_atom, const std::string& host, uint16_t port) {
			aout(self) << "\npassenger: connect_atom received" << endl;
			connecting(self, host, port);
		  }
		};
	}

	void connecting(stateful_actor<state>* self, const std::string& host, uint16_t port)
	{
		// make sure we are not pointing to an old controller
		self->state.current_controller = nullptr;

		// use request().await() to suspend regular behavior until MM responded
		auto mm = self->system().middleman().actor_handle();

		self->request(mm, infinite, connect_atom::value, host, port)
			.await(
				[=](const node_id&, strong_actor_ptr controller,
					const std::set<std::string>& ifs) {
						if (!controller) {
							aout(self) << R"(*** no controller found at ")" << host << R"(":)"
								<< port << endl;
							return;
						}
						if (!ifs.empty()) {
							aout(self) << R"(*** typed actor found at ")" << host << R"(":)"
								<< port << ", but expected an untyped actor " << endl;
							return;
						}
						aout(self) << "*** successfully connected to controller" << endl;
						self->state.current_controller = controller;
						auto controller_hdl = actor_cast<actor>(controller);
						self->monitor(controller_hdl);
						self->send(controller_hdl, elevator::register_passenger_atom::value, self);
						self->become(waiting_for_instruction(self));
						//waiting_for_instruction(self);
				},
				[=](const error& err) {
					aout(self) << R"(*** cannot connect to ")" << host << R"(":)"
						<< port << " => " << self->system().render(err) << endl;
					self->become(unconnected(self));
				}
				);
	}

	behavior waiting_for_instruction(stateful_actor<state>* self)
	{
		auto get_instr = [&]() {
			auto repl = self->spawn(passenger_repl);
			self->send(repl, infinite, elevator::get_instructions_atom::value, self->state.current_floor);
		};

		get_instr();

		return
		{
			[=](elevator::call_atom, int to_floor) { aout(self) << "\ncalling" << endl; get_instr(); },
			[=](elevator::connect_to_controller_atom) { aout(self) << "\nconnecting" << endl; get_instr(); },
			[=](elevator::quit_atom) {anon_send_exit(self, exit_reason::user_shutdown); }
		};
	}

	behavior waiting_for_elevator(stateful_actor<state>* self)
	{
		aout(self) << "passenger: entering waiting_for_elevator";

		return
		{
		  [=](elevator_arrived_atom) {
			aout(self) << "\npassenger: elevator_arrived_atom received" << endl;
			self->become(in_transit(self));
		  }
		};
	}

	behavior in_transit(stateful_actor<state>* self)
	{
		aout(self) << "passenger: entering in_transit";

		return
		{
			[=](destination_arrived_atom)
			{
				aout(self) << "\npassenger: destination_arrived_atom received" << endl;
				//self->become(waiting_for_instruction(self));
				waiting_for_instruction(self);
			}
		};
	}

	//behavior passenger_repl(actor_system& system, const config& cfg, const actor& passenger)


	void start_passenger(actor_system& system, const elevator::config& cfg)
	{
		passenger_usage();

		auto passenger = system.spawn(passenger::init);

		if (!cfg.host.empty() && cfg.port > 0)
			anon_send(passenger, connect_to_controller_atom::value, cfg.host, cfg.port);
		else
			cout << "*** no server received via config, "
			<< R"(please use "connect <host> <port>" before using the calculator)"
			<< endl;

		//passenger_repl(system, cfg, passenger);

		auto self = scoped_actor{ system };
		self->wait_for(passenger);

		//self->request(repl, infinite, get_instructions_atom::value).receive(
		//	[&](int x) {
		//		aout(self) << x << endl;
		//	},
		//	[&](const error& err) {
		//		aout(self) <<  "error: " << err << endl;
		//	}
		//);


		//if (!cfg.host.empty() && cfg.port > 0)
		//	anon_send(passenger, connect_atom::value, cfg.host, cfg.port);
		//else
		//	cout << ">> no elevator controller received via config, "
		//	<< R"(please use "connect <host> <port>" before using the elevator)"
		//	<< endl;
	}





}
