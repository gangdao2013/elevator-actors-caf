#pragma once

#include <iostream>
#include "elevator_actor.hpp"
#include "elevator_fsm.hpp"
#include "caf/all.hpp"

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
	}

	void idle_state::on_enter(elevator_actor& actor)
	{
		actor.on_idle();
	}

	void idle_state::handle_start(elevator_actor& actor)
	{
		if(actor.on_start())
			actor.transition_to_state(elevator_fsm::in_transit);
	}

	void in_transit_state::on_enter(elevator_actor& actor)
	{
		actor.on_in_transit();
	}

	void quitting_state::on_enter(elevator_actor& actor)
	{
		actor.quit();
	}

	void at_waypoint_state::on_enter(elevator_actor& actor)
	{
	}

}
