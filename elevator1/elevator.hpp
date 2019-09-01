#pragma once

#include <string>
#include "caf/all.hpp"
#include "caf/io/all.hpp"


using namespace caf;



namespace elevator
{

	const int FLOOR_MIN = 0;
	const int FLOOR_MAX = 10;


	using call_atom = atom_constant<atom("call")>;
	using register_elevator_atom = atom_constant<atom("r_e")>;
	using register_passenger_atom = atom_constant<atom("r_p_a")>;
	using get_current_passenger_floor_atom = atom_constant<atom("g_c_p_f")>;
	//using repl_prompt_atom = atom_constant<atom("repl_prompt")>;
	using elevator_arrived_atom = atom_constant<atom("e_a")>;
	using destination_arrived_atom = atom_constant<atom("d_a")>;
	using connect_to_controller_atom = atom_constant<atom("c_t_c")>;
	using get_instructions_atom = atom_constant<atom("g_i")>;

	using quit_atom = atom_constant<atom("quit")>;

	class config : public actor_system_config {
	public:
		uint16_t port = 10000;
		std::string host = "localhost";
		bool controller_mode = false;
		bool passenger_mode = true;
		bool elevator_mode = false;

		config() {
			opt_group{ custom_options_, "global" }
				.add(port, "port,p", "set controller port")
				.add(host, "host,H", "set controller host (ignored in controller mode, assumed to be localhost)")
				.add(controller_mode, "controller,c", "enable controller mode")
				.add(passenger_mode, "passenger,p", "enable passenger mode")
				.add(passenger_mode, "elevator,e", "enable elevator mode")
				;
		}
	};
}