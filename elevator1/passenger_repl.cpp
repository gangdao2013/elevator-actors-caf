
#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "string_util.hpp"

#include "passenger_repl.hpp"
#include "elevator.hpp"


using namespace caf;
using namespace elevator;
using namespace std;

namespace passenger
{

	void passenger_repl::usage(scoped_actor &self) 
	{
		//auto self = scoped_actor{ system_ };
		aout(self) << "Usage:" << std::endl
			<< "  quit                  : terminates the program\n"
			<< "  connect <host> <port> : connects to a (remote) lift controller\n"
			<< "  f <to>                : call elevator from current floor to floor <to> (0 is ground floor)" << std::endl << std::flush;
			;
	};

	bool passenger_repl::send_message(message msg) 
	{
		auto self = scoped_actor{ system_ };
		self->send(passenger_, msg);
		return true;
	}

	int passenger_repl::get_current_floor()
	{
		auto self = scoped_actor{ system_ };
		self->request(passenger_, infinite, elevator::get_current_passenger_floor_atom::value).receive
		//self->request(p, infinite, elevator::get_current_passenger_floor_atom::value).receive
		(
			[&](int floor) {current_floor = floor; },
			[&](error& err) { aout(self) << "error: " << self->system().render(err) << std::endl; }
		);
		return current_floor;
	}

	std::string passenger_repl::get_current_state_name()
	{
		auto self = scoped_actor{ system_ };
		std:string state_name;
		self->request(passenger_, infinite, elevator::get_state_name_atom::value).receive
			//self->request(p, infinite, elevator::get_current_passenger_floor_atom::value).receive
			(
				[&](string name) {state_name = name; },
				[&](error& err) { aout(self) << "error: " << self->system().render(err) << std::endl; }
		);
		return state_name;
	}
	

	void passenger_repl::start_repl()
	{

		bool done = false;
		auto self = scoped_actor{ system_ };

		usage(self);
		
		// defining the handler outside the loop is more efficient as it avoids
		// re-creating the same object over and over again
		message_handler eval
		{
			[&](const std::string& cmd)
			{
				if (cmd == "quit")
				{
					done = true;
					self->send(passenger_, quit_atom::value);
				} 
				else if (cmd == "ea") // lift arrives
				{
					self->send(passenger_, elevator_arrived_atom::value);
				}
			},
			[&](std::string& arg0, std::string& arg1, std::string& arg2)
			{
				if (arg0 == "connect")
				{
					char* end = nullptr;
					uint16_t lport = strtoul(arg2.c_str(), &end, 10);
					if (end != arg2.c_str() + arg2.size())
					{
						aout(self) << R"(")" << arg2 << R"(" is not an unsigned integer)" << std::endl;
					}
					else if (lport > std::numeric_limits<uint16_t>::max())
					{
						aout(self) << R"(")" << arg2 << R"(" > )" << std::numeric_limits<uint16_t>::max() << std::endl;						
					}
					else
					{
						std::string host = arg1;
						self->send(passenger_, connect_to_controller_atom::value, host, lport);
					}
				}
			},
			[&](std::string& cmd, std::string& arg1)
			{
				if (cmd == "f")
				{
					auto to_floor = string_util::to_integer(arg1);
					if (to_floor.has_value())
					{
						self->send(passenger_, call_atom::value, to_floor.value());
					}
				}
				else if (cmd == "da") // lift arrives
				{
					auto floor = string_util::to_integer(arg1);
					if (floor.has_value())
					{
						self->send(passenger_, destination_arrived_atom::value, floor.value());
					}
				}
			}
		};

		std::string line;

		aout(self) << "\n(" << get_current_state_name() << ") f " << get_current_floor() << "> " << std::flush;
		while (!done && std::getline(std::cin, line))
		{
			line = string_util::trim(std::move(line)); // ignore leading and trailing whitespaces
			std::vector<std::string> words;
			split(words, line, is_any_of(" "), token_compress_on);

			if(!message_builder(words.begin(), words.end()).apply(eval))
				passenger_repl::usage(self);

			aout(self) << "\n(" << get_current_state_name() << ") f " << get_current_floor() << "> " << std::flush;
		}
		return;
	}
}