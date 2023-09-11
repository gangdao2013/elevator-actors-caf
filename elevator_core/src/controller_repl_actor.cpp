
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

	// controller read/evaluate/print/loop actor - used to listen for and send messages to controllers

	controller_repl_actor::controller_repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id)
		: repl_actor(cfg, target_actor, repl_id)
	{
		set_error_handler([=](scheduled_actor* actor, error& err) -> void
			{
				aout(this) << "controller_repl_actor: error: " << err << std::endl;
			});
	}

	// Usage message
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
		return "[" + get_name() + "][" + get_current_state_name() + "]> ";
	}

	// get the current state of the target controller
	std::string controller_repl_actor::get_current_state_name()
	{

		request(target_actor_, infinite, get_current_state_name_atom_v)
			.await
			(
				[&](string state) { controller_state = state; },
				[&](error& err) { aout(this) << "error: " << err/*this->system().render(err)*/ << std::endl; } // todo
		);
		return controller_state;
	}

	// get the name of the target controller
	std::string controller_repl_actor::get_name()
	{
		if (controller_name != "")
			return controller_name;

		request(target_actor_, infinite, get_name_atom_v)
		.await
		(
			[&](string name) { controller_name = name; },
			[&](error& err) { aout(this) << "error: " << err/*this->system().render(err)*/ << std::endl; }
		);
		return controller_name;
	}

	// The 'Evaluator' in REPL.
	// Note that the repl_actor class uses the returned message_handler in the eval_command function.
	message_handler controller_repl_actor::get_eval()
	{

		message_handler eval
		{
			// quit or help
			[&](const std::string& cmd)
			{
				if (cmd == "quit" || cmd == "q")
				{
					quit = true;
					this->send(target_actor_, quit_atom_v);
				}
				else if (cmd == "help" || cmd == "h") // help
					usage();
			}

		};

		return eval;
	}
}