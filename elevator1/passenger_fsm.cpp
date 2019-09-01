#pragma once

#include <iostream>
#include "passenger_actor.hpp"
#include "passenger_fsm.hpp"
#include "caf/all.hpp"

namespace passenger {

	std::shared_ptr<initialising_state> passenger_state::initalising(new initialising_state);
	std::shared_ptr<disconnected_state> passenger_state::disconnected(new disconnected_state);
	std::shared_ptr<connecting_state> passenger_state::connecting(new connecting_state);
	std::shared_ptr<in_lobby_state> passenger_state::in_lobby(new in_lobby_state);
	std::shared_ptr<in_elevator_state> passenger_state::in_elevator(new in_elevator_state);
	std::shared_ptr<awaiting_instruction_state> passenger_state::awaiting_instruction(new awaiting_instruction_state);
	std::shared_ptr<quitting_state> passenger_state::quitting(new quitting_state);

	void initialising_state::on_enter(passenger_actor& actor)
	{
		actor.initialise();
	}
	void initialising_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		if (event.event_type == passenger_event_type::initialised_ok)
			actor.set_state(passenger_state::disconnected);
		else
			actor.set_state(passenger_state::quitting);
	}

	void disconnected_state::on_enter(passenger_actor& actor)
	{
		
	}

	void disconnected_state::handle_event(passenger_actor& actor, const passenger_event& event)
	{
		switch (event.event_type)
		{
		case passenger_event_type::connect:
			actor.set_state(passenger_state::connecting);
			break;
		case passenger_event_type::quit:
			actor.set_state(passenger_state::quitting);
			break;
		default:
			// stay disconnected
			break;
		}
	}

	void connecting_state::on_enter(passenger_actor& actor)
	{
		actor.connect();
	}

	void connecting_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		switch (event.event_type )
		{
		case passenger_event_type::connected_ok:
			actor.set_state(passenger_state::in_lobby);
			break;
		case passenger_event_type::connection_fail:
			actor.set_state(passenger_state::disconnected);
			break;
		case passenger_event_type::quit:
			actor.set_state(passenger_state::quitting);
			break;
		default:
			actor.set_state(passenger_state::disconnected);
			break;
		}
	}

	void in_lobby_state::on_enter(passenger_actor& actor)
	{
		//actor.get_instruction();
	}

	void in_lobby_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		switch (event.event_type)
		{
		case passenger_event_type::connect:
			actor.set_state(passenger_state::connecting);
			break;
		case passenger_event_type::connection_fail:
			actor.set_state(passenger_state::disconnected);
			break;
		case passenger_event_type::elevator_arrived:
			actor.set_state(passenger_state::in_elevator);
			break;
		case passenger_event_type::quit:
			actor.set_state(passenger_state::quitting);
			break;
		default:
			break;
		}
	}

	void in_elevator_state::on_enter(passenger_actor& actor)
	{
	}

	void in_elevator_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		switch (event.event_type)
		{
		case passenger_event_type::destination_arrived:
			actor.set_state(passenger_state::in_lobby);
			break;
		case passenger_event_type::quit:
			actor.set_state(passenger_state::quitting);
			break;
		default:
			break;
		}
	}

	void awaiting_instruction_state::on_enter(passenger_actor& actor)
	{
		//actor.get_instruction();
	}

	void awaiting_instruction_state::handle_event(passenger_actor& actor, const passenger_event &event)
	{
		switch (event.event_type)
		{
		case passenger_event_type::connect:
			actor.set_state(passenger_state::disconnected);
			break;
		case passenger_event_type::call:
			actor.set_state(passenger_state::in_lobby);
			break;
		case passenger_event_type::quit:
			actor.set_state(passenger_state::quitting);
			break;
		default:
			//actor.get_instruction();
			break;
		}
	}

	void quitting_state::on_enter(passenger_actor& actor)
	{
		actor.quit();
	}

}
