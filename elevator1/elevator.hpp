#pragma once

#include <string>
#include "caf/all.hpp"
#include "caf/io/all.hpp"


using namespace caf;



namespace elevator
{

	const int FLOOR_MIN = 0;
	const int FLOOR_MAX = 10;


	using connect_to_controller_atom = atom_constant<atom("c_t_c")>;
	using get_current_state_name_atom = atom_constant<atom("g_s")>;
	using get_current_floor_atom = atom_constant<atom("g_c_f")>;

	using call_atom = atom_constant<atom("call")>;
	using register_passenger_atom = atom_constant<atom("r_p_a")>;
	using elevator_arrived_atom = atom_constant<atom("e_a")>;
	using destination_arrived_atom = atom_constant<atom("d_a")>;
	
	//using repl_prompt_atom = atom_constant<atom("repl_prompt")>;
	using get_instructions_atom = atom_constant<atom("g_i")>;

	using register_elevator_atom = atom_constant<atom("r_e")>;


	using quit_atom = atom_constant<atom("quit")>;

	class config : public actor_system_config {
	public:
		uint16_t port = 10000;
		std::string host = "localhost";
		bool controller_mode = false;
		bool passenger_mode = false;
		bool elevator_mode = false;

		config() {
			opt_group{ custom_options_, "global" }
				.add(port, "port,p", "set controller port")
				.add(host, "host,H", "set controller host (ignored in controller mode, assumed to be localhost)")
				.add(controller_mode, "controller,C", "enable controller mode")
				.add(passenger_mode, "passenger,P", "enable passenger mode")
				.add(elevator_mode, "elevator,E", "enable elevator mode")
				;
		}
	};
}