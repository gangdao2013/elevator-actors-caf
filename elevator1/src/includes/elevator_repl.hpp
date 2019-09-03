#pragma once
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator.hpp"
#include "elevator_actor.hpp"

namespace elevator
{
	class elevator_repl
	{
	public:

		elevator_repl(actor_system& system, const actor& elevator_) :
			system_{ system }
			, elevator_{ elevator_ }
			, current_floor{ 0 }
		{}

		void start_repl();

		void usage(scoped_actor& self);
		bool send_message(message);
		int get_current_floor();
		std::string get_current_state_name();

	private:

		actor_system& system_;
		const actor& elevator_;
		int current_floor = 0;

	};

}

