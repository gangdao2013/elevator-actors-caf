#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator.hpp"


using namespace caf;

namespace controller 
{
	struct controller_state
	{
		strong_actor_ptr lift;
		strong_actor_ptr passenger;
	};

	behavior controller_actor(stateful_actor<controller_state>* self);
}