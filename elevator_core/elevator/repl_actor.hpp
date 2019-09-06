#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

namespace elevator
{
	class repl_actor : public event_based_actor
	{
	public:

		repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id) : event_based_actor(cfg)
			, target_actor_{ target_actor }
			, repl_id_{repl_id}
		{ }

		behavior make_behavior() override;

		virtual void start_repl();
		virtual void send_message(message);
		virtual void eval_command(std::vector<std::string> command);

		virtual void usage() = 0;
		virtual std::string get_prompt() = 0;
		virtual caf::message_handler get_eval() = 0;

	protected:
		const actor& target_actor_;
		actor command_actor;
		std::string repl_id_;
		bool quit = false;

	};

}



