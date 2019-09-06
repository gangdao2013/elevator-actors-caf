#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/repl_actor.hpp"

namespace controller
{
	class controller_repl_actor : public elevator::repl_actor
	{
	public:

		controller_repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id);
			

		virtual void usage() override;
		virtual std::string get_prompt() override;
		virtual caf::message_handler get_eval();

		std::string controller_name;
		std::string controller_state;


		//int get_current_floor();
		std::string get_current_state_name();
		std::string get_name();


	};

}

