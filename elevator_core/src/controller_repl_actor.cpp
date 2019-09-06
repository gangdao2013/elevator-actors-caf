
#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/string_util.hpp"

#include "elevator/elevator.hpp"
#include "elevator/controller_repl_actor.hpp"


using namespace caf;
using namespace elevator;
using namespace std;

namespace controller
{

	controller_repl_actor::controller_repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id)
		: repl_actor(cfg, target_actor, repl_id)
	{
		set_error_handler([=](scheduled_actor* actor, error& err) -> void
			{
				aout(this) << "controller_repl_actor: error: " << err << std::endl;
			});
	}


	void controller_repl_actor::usage()
	{

		std::string msg =
			"\n" R"(Usage:)" "\n"
			R"(  quit|q                  : terminates the program)" "\n"
			R"(  help|h                  : help, (print this message))" "\n"
			//R"(  connect <host> <port>   : connects to a (remote) lift controller)" "\n"
			//R"(  c <to>                  : call elevator from current floor to floor <to> (0 is ground floor))" "\n"
			;

		aout(this) << msg << std::flush;
	};

	std::string controller_repl_actor::get_prompt()
	{
		//return "[" + get_name() + "][" + get_current_state_name() + "][" + std::to_string(get_current_floor()) + "]> ";
		return "[" + get_name() + "][" + get_current_state_name() + "]> ";
	}

	std::string controller_repl_actor::get_current_state_name()
	{

		request(target_actor_, infinite, elevator::get_current_state_name_atom::value)
			.await
			(
				[&](string state) { controller_state = state; },
				[&](error& err) { aout(this) << "error: " << this->system().render(err) << std::endl; }
		);
		return controller_state;
	}

	std::string controller_repl_actor::get_name()
	{
		if (controller_name != "")
			return controller_name;

		request(target_actor_, infinite, elevator::get_name_atom::value)
		.await
		(
			[&](string name) { controller_name = name; },
			[&](error& err) { aout(this) << "error: " << this->system().render(err) << std::endl; }
		);
		return controller_name;
	}

	message_handler controller_repl_actor::get_eval()
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
				else if (cmd == "help" || cmd == "h") // help
					usage();
			},
			//[&](std::string& cmd, std::string& arg1)
			//{
			//	if (cmd == "c")
			//	{
			//		auto to_floor = string_util::to_integer(arg1);
			//		if (to_floor.has_value())
			//		{
			//			this->send(target_actor_, call_atom::value, to_floor.value());
			//		}
			//	}
			//	else if (cmd == "da") // elevator arrives at destination
			//	{
			//		auto floor = string_util::to_integer(arg1);
			//		if (floor.has_value())
			//		{
			//			this->send(target_actor_, destination_arrived_atom::value, floor.value());
			//		}
			//	}
			//},
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
						this->send(target_actor_, connect_to_controller_atom::value, host, lport);
					}
				}
			}

		};

		return eval;
	}
}