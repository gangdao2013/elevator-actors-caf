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

		friend class elevator_fsm;
		friend class initialising_state;
		friend class disconnected_state;

		friend class waypoint_accepting_state;
		friend class idle_state;
		friend class in_transit_state;
		friend class at_waypoint_state;

		friend class quitting_state;

		public:
			elevator_actor(actor_config& cfg) :
				event_based_actor(cfg)
				, cfg_{ cfg }
			{
				transition_to_state(elevator_fsm::initalising);
			}

			behavior make_behavior() override;

		private:

			actor_config& cfg_;
			std::string controller_host;
			uint16_t controller_port{ 0 };
			strong_actor_ptr controller;
			
			int current_floor = 0;
			int called_floor = 0;

			std::shared_ptr<elevator_fsm> fsm_;
			void transition_to_state(std::shared_ptr<elevator_fsm> state);

			// Actor event handling functions, called by FSM

			bool on_initialise();
			void on_connect(const std::string& host, uint16_t port);
			void on_waypoint_received(int waypoint_floor);
			void on_idle();
			bool on_start();
			void on_in_transit();
			void on_waypoint_arrive(int waypoint_floor);
			void on_quit();

	};
}

