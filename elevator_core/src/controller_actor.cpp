#include <vector>
#include <queue>
#include <string>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/controller_actor.hpp"
#include "elevator/passenger_actor.hpp"
#include "elevator/dispatcher_actor.hpp"

using namespace caf;
using namespace std;

using namespace elevator;
using namespace dispatcher;


namespace controller
{

	// controller_actor is the central controller/supervisor of the network of actors.
	// elevator_actors and passenger_actors register with a controller

	controller_actor::controller_actor(actor_config& cfg) : event_based_actor(cfg)
	{

		// Create dispatcher
		auto x = spawn<dispatcher_actor>(this);
		dispatcher = actor_cast<strong_actor_ptr>(x);

		monitor(dispatcher);

		set_down_handler([=](const down_msg& dm)
		{
			if (dm.source == dispatcher)
			{
				debug_msg("losing connection to dispatcher, respawning");
				//dispatcher = actor_cast<actor>(spawn<dispatcher_actor>(*this));
				monitor(dispatcher);
			}
		});

		//set_default_handler([=](scheduled_actor* actor, message_view& view)
		//	{
		//		aout(this) << "controller: unknown message" << std::endl;
		//		return sec::unexpected_message;
		//	});

		// ignore errors
		set_error_handler([=](scheduled_actor* actor, error& err) -> void
			{
				aout(this) << "controller: error: " << err << std::endl;
			});
	}

	// Overridden make_behaviour function - required for actor classes
	// What is returned is an initialiser list of message handling lambdas that are used as the input
	// to a behavior constructor.
	// CAF matches incoming messages against the types of the handler lambda functions.

	behavior controller_actor::make_behavior()
	{
		return
		{
			[=](quit_atom)
			{
				debug_msg("quit_atom received");
				//this->quit();
				anon_send_exit(this, exit_reason::user_shutdown);
			},
			[=](register_elevator_atom) 
			{
				debug_msg("register_elevator_atom received");
				auto elevator = current_sender();
				send(dispatcher, register_elevator_atom::value, elevator);
			},
			[=](register_passenger_atom) 
			{
				debug_msg("register_passenger_atom received");
				auto passenger = current_sender();
				send(dispatcher, register_passenger_atom::value, passenger);
			},			
			[=](elevator::subscribe_atom sub, std::string subscriber_key, elevator_observable_event_type event_type)
			{
				// subscribe to dispatcher events too
				//delegate(actor_cast<actor>(dispatcher), sub, subscriber_key, event_type);
				add_subscriber(current_sender(), subscriber_key, event_type);
				send(dispatcher, elevator::subscribe_atom::value, current_sender(), subscriber_key, event_type);

				debug_msg("subscribe_atom received");

			},
			[=](get_current_state_name_atom)
			{
				return "running";
			},
			[=](get_name_atom)
			{
				return "controller";
			},
			[&](const exit_msg& ex)
			{
				quit();
			}

		};
	}

	// send message to all debug message subscribers
	void controller_actor::debug_msg(std::string msg)
	{
		string subscriber_msg = "[dispatcher][running]: " + msg;
		for (auto kv : debug_message_subscribers)
		{
			auto recipient = actor_cast<actor>(kv.second);
			send(recipient, message_atom::value, subscriber_msg);
		}

	}

	// register debug message subscribers
	void controller_actor::add_subscriber(strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type)
	{
		// add subscriber to relevant subscriber map
		switch (event_type)
		{
		case elevator_observable_event_type::debug_message:
		{
			// nb: deliberately replace if key is the same, need to destroy any existing ref
			auto existing_ptr = debug_message_subscribers[subscriber_key];
			if (existing_ptr)
			{
				auto handle = actor_cast<actor>(existing_ptr);
				destroy(handle);
			}
			debug_message_subscribers[subscriber_key] = std::move(subscriber);
		}
		break;
		default:
			break;
		};
	}

}

