#pragma once
#include <vector>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/controller_repl_actor.hpp"


using namespace caf;

namespace controller 
{

	// controller_actor is the central controller/supervisor of the network of actors.
	// elevator_actors and passenger_actors register with a controller

	class ELEVATOR_CORE_EXPORT controller_actor : public event_based_actor
	{
	public:

		controller_actor(actor_config& cfg); // : event_based_actor(cfg);


		behavior make_behavior() override;

	protected:

		// dispatcher is spawned by a controller, with passenger floor calls and elevator events
		// being sent to the controller's dispatcher
		strong_actor_ptr dispatcher;

		// add a subscriber actor for debug messages; e.g. controller_repl_actor
		void add_subscriber(const strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type);
		std::map<std::string, strong_actor_ptr> debug_message_subscribers;

		// send a debug message to all the registered subscriber actors
		void debug_msg(std::string msg);


	};

}