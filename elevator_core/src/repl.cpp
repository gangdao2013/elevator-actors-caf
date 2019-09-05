
#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/string_util.hpp"
#include "elevator/elevator.hpp"

#include "elevator/repl_actor.hpp"


using namespace caf;

namespace elevator
{

	behavior repl_actor::make_behavior()
	{
		return 
		{
		[=](quit_atom)
		{
			aout(this) << "\n: quit_atom received" << endl;
			fsm->handle_quit(*this);
		},
		[=](message_atom)
		{

		};
	}

	void repl_actor::send_message(message msg)
	{
		auto self = scoped_actor{ system_ };
		self->send(actor_, msg);
		return;
	}

	void repl_actor::start_repl()
	{

		auto self = scoped_actor{ system_ };
		message_handler eval = get_eval();

		usage();

		std::string line;

		aout(self) << get_prompt() << std::flush;

		// nb: eval_ needs to set quit to true to quit!
		while (!quit && std::getline(std::cin, line))
		{
			line = string_util::trim(std::move(line)); // ignore leading and trailing whitespaces
			std::vector<std::string> words;
			split(words, line, is_any_of(" "), token_compress_on);

			if (words.size() > 0)
			{
				if (!message_builder(words.begin(), words.end()).apply(eval))
					usage();
			}

			aout(self) << get_prompt() << std::flush;
		}
		return;
	}
}