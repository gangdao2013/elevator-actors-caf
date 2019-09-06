
#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/elevator_repl_actor.hpp"
#include "elevator/string_util.hpp"


using namespace caf;

using namespace elevator;
using namespace std;

namespace elevator
{
	elevator_repl_actor::elevator_repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id)
		: repl_actor(cfg, target_actor, repl_id),
		elevator_floor{ 0 }
		, elevator_number{ 0 }
	{
		set_error_handler([=](scheduled_actor* actor, error& err) -> void
			{
				aout(this) << "elevator_repl_actor: error: " << err << std::endl;
			});
	}

	void elevator_repl_actor::usage()
	{
		std::string msg = 
			"\n" R"(Usage:)" "\n"
			R"(  quit|q                  : terminates the program)" "\n"
			R"(  help|h                  : help, (print this message))" "\n"
			R"(  connect|c <host> <port> : connects to a (remote) lift controller)" "\n"
			R"(  w <f>                   : send elevator a waypoint floor <f> (0 is ground floor))" "\n"
			;

		aout(this) << msg << std::flush;
	}
	std::string elevator_repl_actor::get_prompt()
	{
		return "[" + std::to_string(get_elevator_number()) + "][" + get_current_state_name() + "][" + std::to_string(get_current_floor()) + "]> ";
	}

	int elevator_repl_actor::get_elevator_number()
	{
		//if (elevator_number != 0)
		//	return elevator_number;

		request(target_actor_, infinite, elevator::get_elevator_number_atom::value)
		.await
		(
			[&](int number) { elevator_number = number; },
			[&](error& err) { aout(this) << "error: " << system().render(err) << std::endl; }
		);
		return elevator_number;
	}

	int elevator_repl_actor::get_current_floor()
	{
		request(target_actor_, infinite, elevator::get_current_floor_atom::value)
		.await
		(
			[&](int floor) { elevator_floor = floor; },
			[&](error& err) { aout(this) << "error: " << system().render(err) << std::endl; }
		);
		return elevator_floor;
	}

	std::string elevator_repl_actor::get_current_state_name()
	{

		request(target_actor_, infinite, elevator::get_current_state_name_atom::value)
		.await
		(
				[&](string name) { elevator_state = name; },
				[&](error& err) { aout(this) << "error: " << system().render(err) << std::endl; }
		);
		return elevator_state;
	}

	message_handler elevator_repl_actor::get_eval()
	{
		message_handler eval
		{
			[&](const std::string& cmd)
			{
				if (cmd == "quit" || cmd == "q")
				{
					quit = true;
					send(target_actor_, quit_atom::value);
				}
				else if (cmd == "help" || cmd == "h") // help
					usage();
			},
			[&](std::string& cmd, std::string& arg1)
			{
				if (cmd == "w")
				{
					auto waypoint_floor = string_util::to_integer(arg1);
					if (waypoint_floor.has_value())
					{
						send(target_actor_, waypoint_atom::value, waypoint_floor.value());
					}
				}
			},
			[&](std::string& cmd, std::string& arg1, std::string& arg2)
			{
				if (cmd == "connect" || cmd == "c")
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