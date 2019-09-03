
#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/elevator_repl.hpp"
#include "elevator/string_util.hpp"


using namespace caf;
using namespace elevator;
using namespace std;

namespace elevator
{

	void elevator_repl::usage()
	{
		//auto self = scoped_actor{ system_ };
		aout(self) << "Usage:" << std::endl
			<< "  quit|q                  : terminates the program\n"
			<< "  help|                   : help, (print this message)\n"
			<< "  connect|c <host> <port> : connects to a (remote) lift controller\n"
			<< "  w <f>                   : send elevator a waypoint floor <f> (0 is ground floor)" << std::endl << std::flush;
		;
	}
	std::string elevator_repl::get_prompt()
	{
		return "[" + get_name() + "][" + get_current_state_name() + "][" + std::to_string(get_current_floor()) + "]> ";
	}

	string elevator_repl::get_name()
	{
		if (elevator_name != "")
			return elevator_name;

		self->request(actor_, infinite, elevator::get_name_atom::value)
			.receive
			(
				[&](string name) { elevator_name = name; },
				[&](error& err) { aout(self) << "error: " << self->system().render(err) << std::endl; }
		);
		return elevator_name;
	}

	int elevator_repl::get_current_floor()
	{
		self->request(actor_, infinite, elevator::get_current_floor_atom::value)
		.receive
		(
			[&](int floor) { elevator_floor = floor; },
			[&](error& err) { aout(self) << "error: " << self->system().render(err) << std::endl; }
		);
		return elevator_floor;
	}

	std::string elevator_repl::get_current_state_name()
	{

		self->request(actor_, infinite, elevator::get_current_state_name_atom::value)
		.receive
		(
				[&](string name) { elevator_state = name; },
				[&](error& err) { aout(self) << "error: " << self->system().render(err) << std::endl; }
		);
		return elevator_state;
	}

	message_handler elevator_repl::get_eval()
	{
		message_handler eval
		{
			[&](const std::string& cmd)
			{
				if (cmd == "quit" || cmd == "q")
				{
					quit = true;
					self->send(actor_, quit_atom::value);
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
						self->send(actor_, waypoint_atom::value, waypoint_floor.value());
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
						aout(self) << R"(")" << arg2 << R"(" is not a valid port (must be a positive integer))" << std::endl;
					}
					else if (lport > std::numeric_limits<uint16_t>::max())
					{
						aout(self) << R"(")" << arg2 << R"(" > )" << std::numeric_limits<uint16_t>::max() << std::endl;
					}
					else
					{
						std::string host = arg1;
						self->send(actor_, connect_to_controller_atom::value, host, lport);
					}
				}
			}
		};

		return eval;
	}
}