#pragma once
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator.hpp"

namespace passenger
{
	class passenger_repl : public event_based_actor
	{
	public:
		passenger_repl(actor_config& cfg) : event_based_actor(cfg){}
		behavior make_behavior() override;
	
	private:
		message next_instruction(int floor);
	};

}

