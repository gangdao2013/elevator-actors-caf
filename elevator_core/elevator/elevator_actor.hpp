#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <map>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "string_util.hpp"

#include "elevator/elevator.hpp"
#include "elevator/elevator_fsm.hpp"
#include <elevator/elevator_repl_actor.hpp>

#include <cassert>

namespace elevator
{
	enum class elevator_motion
	{
		stationary,
		moving_up,
		moving_down,
	};

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
			elevator_actor(actor_config& cfg, int elevator_number);

			behavior make_behavior() override;

		private:

			actor_config& cfg_;
			int elevator_number;

			std::string controller_host;
			uint16_t controller_port{ 0 };
			strong_actor_ptr controller;

			strong_actor_ptr dispatcher;
			
			int current_floor = 0;
			elevator_motion current_motion = elevator_motion::stationary;

			std::queue<int> waypoint_floors;

			std::shared_ptr<elevator_fsm> fsm;
			void transition_to_state(std::shared_ptr<elevator_fsm> state);

			// Actor event handling functions, called by FSM

			bool on_initialise();
			void on_connect(const std::string& host, uint16_t port);
			void on_waypoint_received(int waypoint_floor);
			void on_idle();
			bool on_start();
			void on_in_transit();
			void on_waypoint_arrive();
			void on_quit();

			void timer_pulse(int seconds);
			void debug_msg(std::string msg);

			// TODO: Destructor; all child objects 

			void add_subscriber(const strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type);
			std::map<std::string, strong_actor_ptr> debug_message_subscribers;


	};
}

