#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator.hpp"

namespace passenger
{
	

	void start_passenger(actor_system& system, const elevator::config& cfg);

}