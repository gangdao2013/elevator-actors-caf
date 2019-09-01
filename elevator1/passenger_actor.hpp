#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator.hpp"
#include "passenger_fsm.hpp"
#include "passenger_repl.hpp"

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
		passenger_actor(actor_config &cfg) : 
			event_based_actor(cfg)
			, cfg_{cfg}
		{ 
			//cfg_ = cfg ;
			//current_state = &passenger_actor::initialising;
			set_state(passenger_state::initalising);
		}

		behavior make_behavior() override;

		void raise_event(const passenger_event& event);

	private:

		actor_config& cfg_;
		std::string controller_host;
		uint16_t controller_port{ 0 };

		strong_actor_ptr controller;
		int current_floor = 0;
		int called_floor = 0;

		std::shared_ptr<passenger_state> state_;

		//// states
		//void initialising(const passenger_event& e);
		//void disconnected(const passenger_event& e);
		////void connecting(const passenger_event& e);
		//void connected(const passenger_event& e);
		//void in_lobby(const passenger_event& e);
		//void in_elevator(const passenger_event& e);

		//

		bool initialise();
		bool connect();
		void quit();
		bool get_instruction();
		void set_state(std::shared_ptr<passenger_state> state);
	};


}
