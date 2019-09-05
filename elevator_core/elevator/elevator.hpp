#pragma once

#include <string>
#include "caf/all.hpp"
#include "caf/io/all.hpp"


using namespace caf;



namespace elevator
{

	const int FLOOR_MIN = 0;
	const int FLOOR_MAX = 10;

	const int FLOOR_TRANSIT_TIME_SEC = 1; // # seconds it takes to transit from one floor to the next, in either direction
	const int AT_WAYPOINT_TIME_SEC = 2; // # seconds the elevator will wait at a waypoint, before moving into in_transit state


	using connect_to_controller_atom = atom_constant<atom("c_t_c")>;
	using get_current_state_name_atom = atom_constant<atom("g_s")>;
	using get_current_floor_atom = atom_constant<atom("g_c_f")>;

	using call_atom = atom_constant<atom("call")>;
	using register_passenger_atom = atom_constant<atom("r_p_a")>;
	using embark_atom = atom_constant<atom("e_a")>;
	using disembark_atom = atom_constant<atom("d_a")>;
	
	using register_elevator_atom = atom_constant<atom("r_e")>;
	using waypoint_atom = atom_constant<atom("w")>;
	using waypoint_arrived_atom = atom_constant<atom("wa")>;

	using timer_atom = atom_constant<atom("t")>;

	using get_name_atom = atom_constant<atom("g_n")>;
	using get_elevator_number_atom = atom_constant<atom("g_e_n")>;
	using elevator_idle_atom = atom_constant<atom("e_i")>;
	using request_elevator_schedule_atom = atom_constant<atom("r_e_s")>;


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