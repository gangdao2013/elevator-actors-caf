#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/repl_actor.hpp"

namespace passenger
{
	class passenger_repl_actor: public elevator::repl_actor
	{
	public:

		passenger_repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id);
		
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

