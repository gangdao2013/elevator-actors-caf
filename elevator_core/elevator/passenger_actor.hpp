#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/elevator.hpp"
#include "elevator/passenger_fsm.hpp"
#include "elevator/passenger_repl_actor.hpp"

namespace passenger
{
	// passenter_actor represents passengers waiting in lobbys or in elevators, moving up and down.
	// They make calls for floors to controller_actors, which in turn delegate to a dispatcher_actor for 
	// scheduling and dispatch.

	// Passengers work hand-in-glove with an embedded finite state machine (FSM) passenger_fsm, to respond
	// appropriately to incoming messages, events, etc., based on the current state.
	// See passenger_fsm.hpp/cpp for more details.

	class passenger_actor : public event_based_actor
	{
		friend class passenger_fsm;
		friend class initialising_state;
		friend class disconnected_state;
		friend class connecting_state;
		friend class in_lobby_state;
		friend class in_elevator_state;
		friend class awaiting_instruction_state;
		friend class quitting_state;

	public:
		passenger_actor(actor_config& cfg, std::string name);

		behavior make_behavior() override;

	protected:

		actor_config& cfg_;
		std::string controller_host;
		uint16_t controller_port{ 0 };
		strong_actor_ptr controller;
		strong_actor_ptr dispatcher;
		
		int current_floor = 0;
		int called_floor = 0;
		std::string name;
		int elevator_number = 0;

		// passenger FSM, represented as a shared_ptr to an object representing the current state
		std::shared_ptr<passenger_fsm> fsm;
		void transition_to_state(std::shared_ptr<passenger_fsm> state);

		// actor event handling functions, called by FSM

		bool on_initialise();
		void on_connect(const std::string& host, uint16_t port);
		void on_call(int from_floor, int to_floor);
		void on_arrive(int arrived_at_floor);
		void on_lobby();
		void on_elevator();
		
		void on_quit();

		// register subscriber actors for debug messages
		void add_subscriber(const strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type);
		std::map<std::string, strong_actor_ptr> debug_message_subscribers;
		
		// send debug message to subscribers
		void debug_msg(std::string msg);
	};


}
