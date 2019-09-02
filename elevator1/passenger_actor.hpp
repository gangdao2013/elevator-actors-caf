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
			transition_to_state(passenger_fsm::initalising);
		}

		behavior make_behavior() override;

	private:

		actor_config& cfg_;
		std::string controller_host;
		uint16_t controller_port{ 0 };

		strong_actor_ptr controller;
		int current_floor = 0;
		int called_floor = 0;

		std::shared_ptr<passenger_fsm> fsm_;
		void transition_to_state(std::shared_ptr<passenger_fsm> state);

		// passenger actor operations

		bool initialise();
		void connect(const std::string& host, uint16_t port);
		
		bool call(int from_floor, int to_floor);
		bool arrive(int arrived_at_floor);

		void into_lobby();
		void into_elevator();
		
		void quit();
	};


}
