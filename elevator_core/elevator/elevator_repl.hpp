#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/repl.hpp"

namespace elevator
{
	class elevator_repl: public elevator::repl
	{
	public:

		elevator_repl(actor_system& system, const actor& actor) : repl(system, actor),
			elevator_floor{ 0 }
		{}

		virtual void usage() override;
		virtual std::string get_prompt() override;
		virtual caf::message_handler get_eval();

		int get_current_floor();
		std::string get_current_state_name();
		std::string get_name();

	protected:
		std::string elevator_state;
		std::string elevator_name;
		int elevator_floor;

	};

}

