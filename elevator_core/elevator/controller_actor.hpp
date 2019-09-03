#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"


using namespace caf;

namespace controller 
{
	struct controller_state
	{
		strong_actor_ptr elevator;
		strong_actor_ptr passenger;
		int passenger_from_floor;
		int passenger_to_floor;
	};

	behavior controller_actor(stateful_actor<controller_state>* self);
}