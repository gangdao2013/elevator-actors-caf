#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/repl_actor.hpp"

namespace controller
{
	// controller read/evaluate/loop actor - presents a command line interface to a controller actor;
	// use this to directly drive an actor
	class ELEVATOR_CORE_EXPORT controller_repl_actor : public elevator::repl_actor
	{
	public:

		controller_repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id);
			

		virtual void usage() override;
		virtual std::string get_prompt() override;
		virtual caf::message_handler get_eval();

		std::string controller_name;
		std::string controller_state;

		// auxilliary functions to get information from the controller_actor
		std::string get_current_state_name();
		std::string get_name();
	};

}

