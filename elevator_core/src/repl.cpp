
#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/string_util.hpp"
#include "elevator/elevator.hpp"

#include "elevator/repl.hpp"


using namespace caf;

namespace elevator
{

	void repl::send_message(message msg)
	{
		auto self = scoped_actor{ system_ };
		self->send(actor_, msg);
		return;
	}

	void repl::start_repl()
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
				// send the vector of strings to the handler for evaluation,
				// handler will attempt to match on each lambda, and run associated code, e.g. sending a message to an actor
				if (!message_builder(words.begin(), words.end()).apply(eval)) 
					usage(); // nothing matched, so prompt the user with help again
			}
			if(!quit)
				aout(self) << get_prompt() << std::flush;
		}
		return;
	}
}