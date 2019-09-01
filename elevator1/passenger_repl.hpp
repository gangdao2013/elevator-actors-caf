#pragma once
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator.hpp"
#include "passenger_actor.hpp"

namespace passenger
{
	class passenger_repl
	{
	public:

		passenger_repl(actor_system& system, const actor& passenger_) : 
			system_{ system }
			, passenger_{ passenger_ }
			, current_floor{ 0 }
		{}
		
		void start_repl();
		
		void usage(scoped_actor&self);
		bool send_message(message);
		int get_current_floor();
		std::string get_current_state_name();

	private:

		actor_system& system_;
		const actor& passenger_;
		int current_floor = 0;

	};

}

