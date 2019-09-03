#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator.hpp"


namespace elevator
{
	class repl
	{
	public:

		repl(actor_system& system, const actor& actor) :
			system_{ system }
			, actor_{ actor }
			, self{system}
		{ 
			//self_ = scoped_actor{ system };
		}

		virtual void start_repl();
		virtual void send_message(message);

		virtual void usage() = 0;
		virtual std::string get_prompt() = 0;
		virtual caf::message_handler get_eval() = 0;

	protected:
		actor_system& system_;
		const actor& actor_;
		scoped_actor self;
		bool quit = false;

	};

}



