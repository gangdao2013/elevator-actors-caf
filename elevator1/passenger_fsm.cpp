#pragma once

#include <iostream>
#include "passenger_actor.hpp"
#include "passenger_fsm.hpp"
#include "caf/all.hpp"

namespace passenger {

	std::shared_ptr<initialising_state> passenger_fsm::initalising(new initialising_state);
	std::shared_ptr<disconnected_state> passenger_fsm::disconnected(new disconnected_state);
	//std::shared_ptr<connecting_state> passenger_state::connecting(new connecting_state);
	std::shared_ptr<in_lobby_state> passenger_fsm::in_lobby(new in_lobby_state);
	std::shared_ptr<in_elevator_state> passenger_fsm::in_elevator(new in_elevator_state);
	//std::shared_ptr<awaiting_instruction_state> passenger_state::awaiting_instruction(new awaiting_instruction_state);
	std::shared_ptr<quitting_state> passenger_fsm::quitting(new quitting_state);

	// Common quit action for all states..
	void passenger_fsm::handle_quit(passenger_actor& actor)
	{
		actor.transition_to_state(passenger_fsm::quitting);
	}

	// Common connect action for all states...assumes connection is dropped, reestablished, then back in lobby

	void passenger_fsm::handle_connect(passenger_actor& actor, std::string host, uint16_t port)
	{
		actor.connect(host, port); // NB: .connect is an async operation, and sets state itself.

		//if(actor.connect(host, port))
		//	actor.transition_to_state(passenger_fsm::in_lobby);
		//else
		//	actor.transition_to_state(passenger_fsm::disconnected);
	}

	void initialising_state::on_enter(passenger_actor& actor)
	{
		if (actor.initialise())
			actor.transition_to_state(passenger_fsm::disconnected);
		else
			actor.transition_to_state(passenger_fsm::quitting);	
	}


	void in_lobby_state::on_enter(passenger_actor& actor)
	{
		actor.into_lobby();
	}

	//void in_lobby_state::handle_connect(passenger_actor& actor, std::string host, uint16_t port)
	//{
	//	if (!actor.connect(host, port))
	//		actor.set_state(passenger_state::disconnected);
	//}

	void in_lobby_state::handle_call(passenger_actor& actor, int from_floor, int to_floor)
	{
		actor.call(from_floor, to_floor);
	}

	void in_lobby_state::handle_elevator_arrived(passenger_actor& actor)
	{
		actor.transition_to_state(passenger_fsm::in_elevator);
	}
		

	void in_elevator_state::on_enter(passenger_actor& actor)
	{
		actor.into_elevator();
	}

	void in_elevator_state::handle_destination_arrived(passenger_actor& actor, int arrived_at_floor)
	{
		actor.arrive(arrived_at_floor);
		actor.transition_to_state(passenger_fsm::in_lobby);
	}


	void quitting_state::on_enter(passenger_actor& actor)
	{
		actor.quit();
	}



}
