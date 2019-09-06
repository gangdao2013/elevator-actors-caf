#pragma once
#include <vector>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"

#include "elevator/controller_repl_actor.hpp"


using namespace caf;

namespace controller 
{




	struct controller_state
	{
		actor& dispatcher;
		actor& elevator;
	};

	class controller_actor : public event_based_actor
	{
	public:

		controller_actor(actor_config& cfg); // : event_based_actor(cfg);


		behavior make_behavior() override;

	protected:

		strong_actor_ptr dispatcher;

		void add_subscriber(const strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type);
		std::map<std::string, strong_actor_ptr> debug_message_subscribers;

		void debug_msg(std::string msg);


	};

}