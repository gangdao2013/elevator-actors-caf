
#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/string_util.hpp"

#include "elevator/elevator.hpp"
#include "elevator/passenger_repl_actor.hpp"


using namespace caf;
using namespace elevator;
using namespace std;

namespace passenger
{
	// Passenger read/evaluate/print/loop actor - used to listen for and send messages to controllers

	passenger_repl_actor::passenger_repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id)
		: repl_actor(cfg, target_actor, repl_id)
	{

		set_default_handler([=](scheduled_actor* actor, message_view& view)
			{
				aout(this) << "passenger_repl_actor: unknown message" << std::endl;
				return sec::unexpected_message;
			});
	}

	// Usage message
	void passenger_repl_actor::usage() 
	{
		std::string msg =
			"\n" R"(Usage:)" "\n"
			R"(  quit|q                  : terminates the program)" "\n"
			R"(  help|h                  : help, (print this message))" "\n"
			R"(  connect <host> <port>   : connects to a (remote) lift controller)" "\n"
			R"(  c <to>                  : call elevator from current floor to floor <to> (0 is ground floor))" "\n"
			;

		aout(this) << msg << std::flush;
	};

	// REPL prompt
	std::string passenger_repl_actor::get_prompt()
	{
		return "[" + get_name() + "][" + get_current_state_name() + "][" + std::to_string(get_current_floor()) + "]> ";
	}

	// Get current floor from passenger actor - used in prompt
	int passenger_repl_actor::get_current_floor()
	{
		request(target_actor_, infinite, elevator::get_current_floor_atom::value)
		.await
		(
			[&](int floor) {passenger_floor = floor; },
			[&](error& err) { aout(this) << "error: " << this->system().render(err) << std::endl; }
		);
		return passenger_floor;
	}

	// Get current state (e.g. in lobby/in elevator) from passenger actor - used in prompt
	std::string passenger_repl_actor::get_current_state_name()
	{
		request(target_actor_, infinite, elevator::get_current_state_name_atom::value)
		.await
		(
			[&](string state) {passenger_state = state; },
			[&](error& err) { aout(this) << "error: " << this->system().render(err) << std::endl; }
		);
		return passenger_state;
	}

	// Get name from passenger actor - used in prompt
	std::string passenger_repl_actor::get_name()
	{
		if (passenger_name != "")
			return passenger_name;

		this->request(target_actor_, infinite, elevator::get_name_atom::value)
		.await
		(
			[&](string name) { passenger_name = name; },
			[&](error& err) { aout(this) << "error: " << this->system().render(err) << std::endl; }
		);
		return passenger_name;
	}

	// Get eval/message_handler - used by REPL to evaluate commands
	message_handler passenger_repl_actor::get_eval()
	{
		message_handler eval
		{
			[&](const std::string& cmd)
			{
				if (cmd == "quit" || cmd == "q")
				{
					quit = true;
					this->send(target_actor_, quit_atom::value);
				} 
				else if (cmd == "ea") // lift arrives
				{
					send(target_actor_, embark_atom::value);
				} else if (cmd == "help" || cmd == "h") // help
					usage();
			},
			[&](std::string& cmd, std::string& arg1)
			{
				// call for elevator from current floor to arg 1, e.g. "c 4", call for elevator to floor 4
				if (cmd == "c")
				{
					auto to_floor = string_util::to_integer(arg1);
					if (to_floor.has_value())
					{
						this->send(target_actor_, call_atom::value, to_floor.value());
					}
				}
				else if (cmd == "da") // elevator arrives at destination
				{
					auto floor = string_util::to_integer(arg1);
					if (floor.has_value())
					{
						send(target_actor_, disembark_atom::value, floor.value());
					}
				}
			},
			[&](std::string& cmd, std::string& arg1, std::string& arg2)
			{
				if (cmd == "connect")
				{
					char* end = nullptr;
					uint16_t lport = strtoul(arg2.c_str(), &end, 10);
					if (end != arg2.c_str() + arg2.size())
					{
						aout(this) << R"(")" << arg2 << R"(" is not a valid port (must be a positive integer))" << std::endl;
					}
					else if (lport > std::numeric_limits<uint16_t>::max())
					{
						aout(this) << R"(")" << arg2 << R"(" > )" << std::numeric_limits<uint16_t>::max() << std::endl;
					}
					else
					{
						std::string host = arg1;
						send(target_actor_, connect_to_controller_atom::value, host, lport);
					}
				}
			}

		};

		return eval;
	}
}