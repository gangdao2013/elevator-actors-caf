#pragma once

#include <iostream>
#include "elevator_actor.hpp"
#include "elevator_fsm.hpp"
#include "caf/all.hpp"

namespace elevator 
{

	std::shared_ptr<initialising_state> elevator_state::initalising(new initialising_state);
	std::shared_ptr<disconnected_state> elevator_state::disconnected(new disconnected_state);
	std::shared_ptr<connecting_state> elevator_state::connecting(new connecting_state);
	std::shared_ptr<idle_state> elevator_state::idle(new idle_state);
	std::shared_ptr<in_transit_state> elevator_state::in_transit(new in_transit_state);
	//std::shared_ptr<awaiting_instruction_state> elevator_state::awaiting_instruction(new awaiting_instruction_state);
	std::shared_ptr<quitting_state> elevator_state::quitting(new quitting_state);

	void initialising_state::on_enter(elevator_actor& actor)
	{
		actor.initialise();
	}

	void initialising_state::handle_event(elevator_actor& actor, const elevator_event& event)
	{
		if (event.event_type == elevator_event_type::initialised_ok)
			actor.set_state(elevator_state::disconnected);
		else
			actor.set_state(elevator_state::quitting);
	}

	void disconnected_state::on_enter(elevator_actor& actor)
	{

	}

	void disconnected_state::handle_event(elevator_actor& actor, const elevator_event& event)
	{
		switch (event.event_type)
		{
		case elevator_event_type::connect:
			actor.set_state(elevator_state::connecting);
			break;
		case elevator_event_type::quit:
			actor.set_state(elevator_state::quitting);
			break;
		default:
			// stay disconnected
			break;
		}
	}

	void connecting_state::on_enter(elevator_actor& actor)
	{
		actor.connect();
	}

	void connecting_state::handle_event(elevator_actor& actor, const elevator_event& event)
	{
		switch (event.event_type)
		{
		case elevator_event_type::connected_ok:
			actor.set_state(elevator_state::idle);
			break;
		case elevator_event_type::connection_fail:
			actor.set_state(elevator_state::disconnected);
			break;
		case elevator_event_type::quit:
			actor.set_state(elevator_state::quitting);
			break;
		default:
			actor.set_state(elevator_state::disconnected);
			break;
		}
	}

	void idle_state::on_enter(elevator_actor& actor)
	{
		//actor.get_instruction();
	}

	void idle_state::handle_event(elevator_actor& actor, const elevator_event& event)
	{
		switch (event.event_type)
		{
		case elevator_event_type::connect:
			actor.set_state(elevator_state::connecting);
			break;
		case elevator_event_type::connection_fail:
			actor.set_state(elevator_state::disconnected);
			break;
		//case elevator_event_type::elevator_arrived:
		//	actor.set_state(elevator_state::in_elevator);
		//	break;
		case elevator_event_type::quit:
			actor.set_state(elevator_state::quitting);
			break;
		default:
			break;
		}
	}

	void in_transit_state::on_enter(elevator_actor& actor)
	{
	}

	void in_transit_state::handle_event(elevator_actor& actor, const elevator_event& event)
	{
		switch (event.event_type)
		{
		case elevator_event_type::destination_arrived:
			actor.set_state(elevator_state::idle);
			break;
		case elevator_event_type::quit:
			actor.set_state(elevator_state::quitting);
			break;
		default:
			break;
		}
	}

	void quitting_state::on_enter(elevator_actor& actor)
	{
		actor.quit();
	}

	//void awaiting_instruction_state::on_enter(elevator_actor& actor)
	//{
	//	//actor.get_instruction();
	//}

	//void awaiting_instruction_state::handle_event(elevator_actor& actor, const elevator_event& event)
	//{
	//	switch (event.event_type)
	//	{
	//	case elevator_event_type::connect:
	//		actor.set_state(elevator_state::disconnected);
	//		break;
	//	case elevator_event_type::call:
	//		actor.set_state(elevator_state::in_lobby);
	//		break;
	//	case elevator_event_type::quit:
	//		actor.set_state(elevator_state::quitting);
	//		break;
	//	default:
	//		//actor.get_instruction();
	//		break;
	//	}
	//}



}
