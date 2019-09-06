#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/repl_actor.hpp"

namespace elevator
{
	class elevator_repl_actor: public elevator::repl_actor
	{
	public:

		elevator_repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id);

		virtual void usage() override;
		virtual std::string get_prompt() override;
		virtual caf::message_handler get_eval();

		int get_current_floor();
		std::string get_current_state_name();
		int get_elevator_number();

	protected:
		std::string elevator_state;
		int elevator_number;
		int elevator_floor;

	};

}
