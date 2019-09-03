
#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/string_util.hpp"

#include "elevator/passenger_repl.hpp"
#include "elevator/elevator.hpp"


using namespace caf;
using namespace elevator;
using namespace std;

namespace passenger
{

	void passenger_repl::usage() 
	{
		aout(self) << "Usage:" << std::endl
			<< "  quit|q                  : terminates the program\n"
			<< "  connect|c <host> <port> : connects to a (remote) lift controller\n"
			<< "  c <to>                  : call elevator from current floor to floor <to> (0 is ground floor)" << std::endl << std::flush;
			;
	};

	std::string passenger_repl::get_prompt()
	{
		return "[" + get_name() + "][" + get_current_state_name() + "][" + std::to_string(get_current_floor()) + "]> ";
	}

	int passenger_repl::get_current_floor()
	{
		self->request(actor_, infinite, elevator::get_current_floor_atom::value).receive
		(
			[&](int floor) {passenger_floor = floor; },
			[&](error& err) { aout(self) << "error: " << self->system().render(err) << std::endl; }
		);
		return passenger_floor;
	}

	std::string passenger_repl::get_current_state_name()
	{

		self->request(actor_, infinite, elevator::get_current_state_name_atom::value).receive
		(
			[&](string state) {passenger_state = state; },
			[&](error& err) { aout(self) << "error: " << self->system().render(err) << std::endl; }
		);
		return passenger_state;
	}


	std::string passenger_repl::get_name()
	{
		if (passenger_name != "")
			return passenger_name;

		self->request(actor_, infinite, elevator::get_name_atom::value).receive
		(
			[&](string name) { passenger_name = name; },
			[&](error& err) { aout(self) << "error: " << self->system().render(err) << std::endl; }
		);
		return passenger_name;
	}

	message_handler passenger_repl::get_eval()
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
				else if (cmd == "ea") // lift arrives
				{
					self->send(actor_, elevator_arrived_atom::value);
				}
			},
			[&](std::string& cmd, std::string& arg1)
			{
				if (cmd == "c")
				{
					auto to_floor = string_util::to_integer(arg1);
					if (to_floor.has_value())
					{
						self->send(actor_, call_atom::value, to_floor.value());
					}
				}
				else if (cmd == "da") // lift arrives
				{
					auto floor = string_util::to_integer(arg1);
					if (floor.has_value())
					{
						self->send(actor_, destination_arrived_atom::value, floor.value());
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