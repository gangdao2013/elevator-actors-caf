#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "string_util.hpp"

#include "elevator.hpp"
#include "elevator_fsm.hpp"
#include "elevator_repl.hpp"

#include <cassert>

namespace elevator
{
	class elevator_actor : public event_based_actor
	{

		friend class passenger_fsm;
		friend class initialising_state;
		friend class disconnected_state;
		friend class connecting_state;
		friend class in_lobby_state;
		friend class idle_state;
		friend class in_transit_state;
		friend class quitting_state;

		public:
			elevator_actor(actor_config& cfg) :
				event_based_actor(cfg)
				, cfg_{ cfg }
			{
				set_state(elevator_state::initalising);
			}

			behavior make_behavior() override;
			void raise_event(const elevator_event& event);

		private:

			actor_config& cfg_;
			std::string controller_host;
			uint16_t controller_port{ 0 };

			strong_actor_ptr controller;
			int current_floor = 0;
			int called_floor = 0;

			std::shared_ptr<elevator_state> state_;
			void set_state(std::shared_ptr<elevator_state> state);

			void initialise();
			void connect();
			void quit();

	};
}

