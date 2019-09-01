#pragma once

#include <iostream>
#include "passenger_actor.hpp"
#include "passenger_fsm.hpp"
#include "caf/all.hpp"

namespace passenger {

	void initialising_state::on_enter(passenger_actor& actor)
	{
		if(actor.initialise())
			this->handle_event(actor, passenger_event{ actor, passenger_event_type::initialised_ok });

	}

	passenger_state* initialising_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		if (event.event_type == passenger_event_type::initialised_ok)
			return &passenger_state::disconnected;
		else
			return this;	
	}

	void disconnected_state::on_enter(passenger_actor& actor)
	{
		if(actor.connect())
			this->handle_event(actor, passenger_event{ actor, passenger_event_type::connected_ok });
		else
			this->handle_event(actor, passenger_event{ actor, passenger_event_type::connection_fail});
	}

	passenger_state* disconnected_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		switch (event.event_type )
		{
		case passenger_event_type::connected_ok:
			return &passenger_state::in_lobby;
			break;
		case passenger_event_type::connection_fail:
			return &passenger_state::awaiting_instruction;
			break;
		case passenger_event_type::quit:
			return &passenger_state::quitting;
			break;
		default:
			return this;
		}
	}

	void in_lobby_state::on_enter(passenger_actor& actor)
	{
		actor.get_instruction();
	}

	passenger_state* in_lobby_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		switch (event.event_type)
		{
		case passenger_event_type::connection_fail:
			return &passenger_state::awaiting_instruction;
			break;
		case passenger_event_type::elevator_arrived:
			return &passenger_state::in_elevator;
			break;
		case passenger_event_type::quit:
			return &passenger_state::quitting;
			break;
		default:
			return this;
		}
	}

	void in_elevator_state::on_enter(passenger_actor& actor)
	{
	}

	passenger_state* in_elevator_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		return this;
	}

	void awaiting_instruction_state::on_enter(passenger_actor& actor)
	{

	}

	passenger_state* awaiting_instruction_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		return this;
	}

	void quitting_state::on_enter(passenger_actor& actor)
	{
		actor.quit();
	}

}
