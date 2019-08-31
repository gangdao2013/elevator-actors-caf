#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator.hpp"


using namespace caf;

namespace controller 
{
	void start_controller(actor_system& system, const elevator::config& cfg);
}