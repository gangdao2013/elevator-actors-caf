
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
	// next_command_actor is used to asynchronously get commands from the user.
	// This is so we can wait on commands without blocking, enabling (for example) incoming messages to be displayed.

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

	// repl_actors are pretty simple
	// They just start, wait for commands from next_command_actors, evaluate commands and display results
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

	// Start
	void repl_actor::start_repl()
	{
		// NB: get_eval() is overriden by child classes, returns a handler specific to that repl class and target_actor
		message_handler eval = get_eval(); 
		// this lets the target_actor know we want (debug) messages
		send(target_actor_, subscribe_atom::value, repl_id_, elevator_observable_event_type::debug_message); 

		usage(); // overridden by child class
		command_actor = spawn(next_command_actor);
		send(command_actor, get_prompt());
	}

	// Evaluate a command (represented as a vector of strings) received from the command_actor
	void repl_actor::eval_command(std::vector<std::string> command)
	{
		message_handler eval = get_eval(); // handler is specific to each child class
		if (command.size() > 0)
		{
			// The .apply(eval) method evaluates eval/message_handler against the constructed message object (from the commands)
			// It's a handy reuse of the CAF message handling & matching code for our REPL purposes.
			if (!message_builder(command.begin(), command.end()).apply(eval)) 
				usage();
		}
	}
}