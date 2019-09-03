#pragma once

#include <iostream>
#include "elevator_actor.hpp"
#include "elevator_fsm.hpp"
#include "caf/all.hpp"
#include "caf/io/all.hpp"

using namespace caf;

namespace elevator
{

	std::shared_ptr<initialising_state> elevator_fsm::initalising(new initialising_state);
	std::shared_ptr<disconnected_state> elevator_fsm::disconnected(new disconnected_state);
	std::shared_ptr<idle_state> elevator_fsm::idle(new idle_state);
	std::shared_ptr<in_transit_state> elevator_fsm::in_transit(new in_transit_state);
	std::shared_ptr<quitting_state> elevator_fsm::quitting(new quitting_state);

	// Common quit action for all states..
	void elevator_fsm::handle_quit(elevator_actor& actor)
	{
		actor.transition_to_state(elevator_fsm::quitting);
	}

	// Common connect action for all states...assumes connection is dropped, reestablished, then back in lobby

	void elevator_fsm::handle_connect(elevator_actor& actor, std::string host, uint16_t port)
	{
		actor.on_connect(host, port); // NB: .connect is an async operation, and sets state itself.
	}


	void initialising_state::on_enter(elevator_actor& actor)
	{
		if (actor.on_initialise())
			actor.transition_to_state(elevator_fsm::disconnected);
		else
			actor.transition_to_state(elevator_fsm::quitting);
	}

	// common waypoint acceptance action for accepting states
	void waypoint_accepting_state::handle_waypoint_received(elevator_actor& actor, int waypoint_floor)
	{
		actor.on_waypoint_received(waypoint_floor);
		actor.fsm->handle_start(actor);
	}

	void idle_state::on_enter(elevator_actor& actor)
	{
		actor.on_idle();
	}

	void idle_state::handle_start(elevator_actor& actor)
	{
		if (actor.on_start())
			actor.transition_to_state(elevator_fsm::in_transit);
	}

	void in_transit_state::on_enter(elevator_actor& actor)
	{
		actor.on_in_transit();
		// start in transit timer cycle, in_transit will check 
		actor.timer_pulse(elevator::FLOOR_TRANSIT_TIME_SEC);
	}

	void in_transit_state::handle_timer(elevator_actor& actor)
	{
		// decrease/increase current floor, check against waypoint, transition to 
		// at_waypoint, or continue in_transit

		// for now just go to idle once waypoint is hit.
		if (actor.waypoint_floors.size() == 0)
		{
			actor.current_motion = elevator_motion::stationary;
			actor.transition_to_state(elevator_fsm::idle);
			return;
		}

		int next_waypoint_floor = actor.waypoint_floors.front();

		if (next_waypoint_floor > actor.current_floor)
			actor.current_motion = elevator_motion::moving_up;
		else if (next_waypoint_floor < actor.current_floor)
			actor.current_motion = elevator_motion::moving_down;
		else
			actor.current_motion = elevator_motion::stationary;

		switch (actor.current_motion)
		{
		case elevator_motion::stationary:
			// arrived at waypoint
			actor.debug_msg("elevator: stopped at floor: " + std::to_string(actor.current_floor) + "\n");
			actor.waypoint_floors.pop();
			// if no more waypoints then go to idle, else go into at waypoint
			if (actor.waypoint_floors.size() == 0)
				actor.transition_to_state(elevator_fsm::idle);
			else
				actor.transition_to_state(elevator_fsm::at_waypoint);
			break;
		case elevator_motion::moving_up:
			if (actor.current_floor < elevator::FLOOR_MAX)
			{
				if (actor.current_floor < next_waypoint_floor) // TODO: next_waypoint will be a call to controller??
				{
					actor.debug_msg("elevator: passing floor: " + std::to_string(actor.current_floor) + "\n");
					actor.current_floor++;
					actor.timer_pulse(elevator::FLOOR_TRANSIT_TIME_SEC);
				}
				else
				{
					// at waypoint
					actor.current_floor = next_waypoint_floor;
					actor.debug_msg("elevator: stopping at floor: " + std::to_string(actor.current_floor) + "\n");
					actor.waypoint_floors.pop();
					actor.transition_to_state(elevator_fsm::idle);
				}
			}
			else
			{
				actor.current_floor = elevator::FLOOR_MAX;
				actor.waypoint_floors.pop();
				actor.transition_to_state(elevator_fsm::idle);
			}
			break;
		case elevator_motion::moving_down:
			if (actor.current_floor > elevator::FLOOR_MIN)
			{
				if (actor.current_floor > next_waypoint_floor)
				{
					actor.debug_msg("elevator: passing floor: " + std::to_string(actor.current_floor) + "\n");
					actor.current_floor--;
					actor.timer_pulse(elevator::FLOOR_TRANSIT_TIME_SEC);
				}
				else
				{
					// at waypoint
					actor.current_floor = next_waypoint_floor;
					actor.debug_msg("elevator: stopping at floor: " + std::to_string(actor.current_floor) + "\n");
					actor.waypoint_floors.pop();
					actor.transition_to_state(elevator_fsm::idle);
				}
			}
			else
			{
				actor.current_floor = elevator::FLOOR_MIN;
				actor.waypoint_floors.pop();
				actor.transition_to_state(elevator_fsm::idle);
			}
			break;
		default: // TODO: default timer behaviour here??
			break;
		}
	}

	void quitting_state::on_enter(elevator_actor& actor)
	{
		actor.quit();
	}

	void at_waypoint_state::on_enter(elevator_actor& actor)
	{
	}

}
