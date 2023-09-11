#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

namespace elevator
{
	// Read/evaluate/print (REPL) loop actor - abstract class with base behaviour for specific REPLs.

	// It can be used to monitor the behaviour of an actor, as well as send it commands directly - handy for development & testing.

	class ELEVATOR_CORE_EXPORT repl_actor : public event_based_actor
	{
	public:

		repl_actor(actor_config& cfg, const actor& target_actor, std::string repl_id) : event_based_actor(cfg)
			, target_actor_{ target_actor }
			, repl_id_{repl_id}
		{ }

		behavior make_behavior() override;

		virtual void start_repl();				// Start the L in REPL
		virtual void send_message(message);		// Send a specific message object to the target_actor
		virtual void eval_command(std::vector<std::string> command);	// Evaluate the command received from command_actor


		// These functions need to be supplied by inheriting classes:
		virtual void usage() = 0;						// Usage message
		virtual std::string get_prompt() = 0;			// Prompt string for the REPL command line
		virtual caf::message_handler get_eval() = 0;	// Eval message_handler object for the E in REPL - see child classes for details

	protected:
		const actor& target_actor_;		// The actor we are REPLing with
		actor command_actor;			// command_actor is used to get next input command string asynchronously, without blocking incoming messages from target_actor
		std::string repl_id_;			// Used to register this REPL in target_actor::register_subscriber
		bool quit = false;				// When set true the REPL will quit

	};

}



