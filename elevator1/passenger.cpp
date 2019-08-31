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

		// the client queues pending tasks
	struct state {
		strong_actor_ptr current_controller;
		strong_actor_ptr repl;
		//const actor& repl;
		int current_floor = 0;
		int called_floor = 0;
	};

	void passenger_usage() {
		cout << "Usage:" << endl
			<< "  quit                  : terminates the program\n" //<< endl
			<< "  connect <host> <port> : connects to a (remote) lift controller\n" //<< endl
			<< "  f <to>                : call elevator from current floor to floor <to> (0 is ground floor)"s << std::endl
			;
	};

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


	// starting point of our passenger FSM
	behavior init(stateful_actor<state>* self)
	{

		// transition to `unconnected` on elevator controller failure/shutdown
		// set the handler if we lose connection to elevator controller 
		self->set_down_handler([=](const down_msg& dm) {
			if (dm.source == self->state.current_controller) {
				aout(self) << "\npassenger: lost connection to elevator controller, please reconnect or quit" << endl;
				self->state.current_controller = nullptr;
				self->become(unconnected(self));
			}
			});

		// now transition to 'unconnected'
		return unconnected(self);
	}

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
	behavior passenger_repl(event_based_actor* self) //, const actor &passenger)
	{


		//auto current_floor = [&]() -> int {
		//	auto self = scoped_actor { system };
		//	self->request(passenger, infinite, elevator::get_current_passenger_floor_atom::value).receive(
		//		[&](int floor) {return floor; },
		//		[&](error& err) { aout(self) << "error: " << self->system().render(err) << endl; return 0; }
		//	);
		//};
		auto get_next_instruction = [&](int floor) -> caf::message
		{

			bool done = false;

			// defining the handler outside the loop is more efficient as it avoids
			// re-creating the same object over and over again
			message_handler eval
			{
				[&](const string& cmd) -> caf::optional<message>
				{
					if (cmd != "quit")
						return {};
					done = true;
					return make_message(quit_atom::value);
				},
				[&](string& arg0, string& arg1, string& arg2) -> caf::optional<message>
				{
					if (arg0 == "connect")
					{
						char* end = nullptr;
						auto lport = strtoul(arg2.c_str(), &end, 10);
						if (end != arg2.c_str() + arg2.size())
						{
							cout << R"(")" << arg2 << R"(" is not an unsigned integer)" << endl;
							return {};
						}
						else if (lport > std::numeric_limits<uint16_t>::max())
						{
							cout << R"(")" << arg2 << R"(" > )" << std::numeric_limits<uint16_t>::max() << endl;
							return {};
						}
						else
							return make_message(connect_to_controller_atom::value, arg1, lport);
					}
					return {};
				},
				[&](string& arg0, string& arg1) -> caf::optional<message>
				{
					if (arg0 == "f")
					{
						auto to_floor = string_util::to_integer(arg1);
						if (to_floor.has_value())
						{
							//self->state.called_floor = to_floor; // .value();
							//cout << "\nCalling elevator from floor: " << self->state.current_floor << " to floor: "; // << to_floor.value() << endl;
							//self->send(self->state.current_controller, elevator::call_atom::value, self, self->state.current_floor, self->state.called_floor);
							//self->become(waiting_for_elevator);
							return make_message(call_atom::value, to_floor.value());
						}
					}
					return {};
				}
			};

			string line;
			//int floor = current_floor();
			cout << "\nf " << floor << "> " << endl;
			while (!done)
			{
				std::getline(std::cin, line);
				line = string_util::trim(std::move(line)); // ignore leading and trailing whitespaces
				std::vector<string> words;
				split(words, line, is_any_of(" "), token_compress_on);

				auto result = message_builder(words.begin(), words.end()).apply(eval);

				if (!result || result.value().size() == 0)
					passenger_usage();
				else
					return result.value();
				cout << "\nf " << floor << "> " << endl;
			}
		};

		return
		{
			[&](elevator::get_instructions_atom, int current_floor) -> message {return get_next_instruction(current_floor); }
		};
	}

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
