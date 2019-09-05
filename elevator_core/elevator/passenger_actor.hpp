#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/elevator.hpp"
#include "elevator/passenger_fsm.hpp"
#include "elevator/passenger_repl_actor.hpp"

namespace passenger
{

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
		passenger_actor(actor_config &cfg, std::string name) : 
			event_based_actor(cfg)
			, cfg_{cfg}
			, name{ name }
		{ 
			transition_to_state(passenger_fsm::initalising);
		}

		behavior make_behavior() override;

	protected:

		actor_config& cfg_;
		std::string controller_host;
		uint16_t controller_port{ 0 };
		strong_actor_ptr controller;
		
		int current_floor = 0;
		int called_floor = 0;
		std::string name;

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

		void debug_msg(std::string msg);

		void add_subscriber(const strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type);
		std::map<std::string, strong_actor_ptr> debug_message_subscribers;

	};


}
