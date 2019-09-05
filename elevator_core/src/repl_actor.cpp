
#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/string_util.hpp"
#include "elevator/elevator.hpp"

#include "elevator/repl_actor.hpp"


using namespace caf;
using namespace elevator;

namespace elevator
{

	behavior next_command_actor(event_based_actor* self)
	{
		return
		{
			[=](std::string prompt)
			{

				std::cout << "\n" << prompt << std::flush;
				std::string line;
				std::getline(std::cin, line);
				line = string_util::trim(std::move(line)); // ignore leading and trailing whitespaces
				std::vector<std::string> words;
				split(words, line, is_any_of(" "), token_compress_on);
				self->send(self->current_sender(), command_atom::value, words);
			}
		};
	}

	behavior repl_actor::make_behavior()
	{
		return
		{
			[=](quit_atom)
			{
				aout(this) << "\n:::quit_atom received" << std::endl;
				anon_send_exit(this, exit_reason::user_shutdown);
			},
			[=](start_atom)
			{
				aout(this) << "\n:::start_atom received" << std::endl;
				start_repl();
			},
			[=](message_atom, std::string message)
			{
				aout(this) << std::endl << message << std::endl << std::flush;
			},
			[=](command_atom, std::vector<std::string> command)
			{
				eval_command(command);
				if (!quit)
					send(command_actor, get_prompt());
				else
					send(this, quit_atom::value); // nb: eval would have sent quit to target_actor_ already
			}
		};
	}

	void repl_actor::send_message(message msg)
	{
		send(target_actor_, msg);
		return;
	}

	void repl_actor::start_repl()
	{

		message_handler eval = get_eval();
		send(target_actor_, subscribe_atom::value, repl_id_, elevator_observable_event_type::debug_message);

		usage();
		command_actor = spawn(next_command_actor);
		send(command_actor, get_prompt());
	}

	void repl_actor::eval_command(std::vector<std::string> command)
	{
		message_handler eval = get_eval();
		if (command.size() > 0)
		{
			if (!message_builder(command.begin(), command.end()).apply(eval))
				usage();
		}
	}
}