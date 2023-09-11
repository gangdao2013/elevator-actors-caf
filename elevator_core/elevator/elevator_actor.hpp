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

	// elevator_actor represents elevators moving up and down.
	// They are scheduled by dispatcher_actor and class scheduler.
	// Elevators are 'tasked' with schedules of floor waypoints; the dispatcher maintains lists of pickups and dropoffs
	// for each floor in any schedule.

	// Elevators work hand-in-glove with an embedded finite state machine (FSM) elevator_fsm, to respond
	// appropriately to incoming messages, events, etc., based on the current state.
	// See elevator_fsm.cpp for more details.

	class ELEVATOR_CORE_EXPORT elevator_actor : public event_based_actor
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

			// dispatcher is created by controller and set via message
			strong_actor_ptr dispatcher;
			
			int current_floor = 0;
			elevator_motion current_motion = elevator_motion::stationary;

			// The current taskig of waypoints. Note that the elevator visits them in queue order,
			// which would ordinarily be increasing/decreasing order. 
			std::queue<int> waypoint_floors;

			// The embedded FSM - realised as a shared_ptr to an object that derives from elevator_fsm class.
			// The current state is represented by the object that the fsm pointer points to.
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

			// timer is used to simulate travel between floors and pauses at waypoints
			void timer_pulse(int seconds);

			// TODO: Destructor; all child objects 

			// Add subscriber actors for debug messages, e.g. REPL actors.
			void add_subscriber(const strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type);
			std::map<std::string, strong_actor_ptr> debug_message_subscribers;
			// Send debug message to all subscribers
			void debug_msg(std::string msg);


	};
}

