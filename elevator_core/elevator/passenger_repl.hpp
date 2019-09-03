#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/repl.hpp"

namespace passenger
{
	class passenger_repl: public elevator::repl
	{
	public:

		passenger_repl(actor_system& system, const actor& actor) : repl(system, actor)
		{}
		
		virtual void usage() override;
		virtual std::string get_prompt() override;
		virtual caf::message_handler get_eval();
	

		int get_current_floor();
		std::string get_current_state_name();
		std::string get_name();

	protected:
		std::string passenger_state;
		std::string passenger_name;
		int passenger_floor = 0;

	};

}

