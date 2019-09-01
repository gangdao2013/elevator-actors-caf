#include <vector>
#include <string>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator.hpp"
#include "controller.hpp"
#include "passenger_actor.hpp"

using namespace caf;
using namespace std;

using namespace elevator;


namespace controller
{
	struct controller_state
	{
		strong_actor_ptr lift;
		strong_actor_ptr passenger;
	};

	const int FLOOR_MIN = 0;
	const int FLOOR_MAX = 10;

	behavior controller_actor(stateful_actor<controller_state>* self)
	{
		return
		{
			[=](register_elevator_atom, strong_actor_ptr lift) {
				aout(self) << "\nregister_elevator_atom received" << endl;
				self->state.lift = lift;
				self->link_to(lift);
			},
			[=](register_passenger_atom, strong_actor_ptr passenger) {
				aout(self) << "\nregister_passenger_atom received" << endl;
				self->state.passenger = passenger;
				self->link_to(passenger);
			},
			[=](call_atom) {
				//self->send(self->state.lift, up_atom::value);
				aout(self) << "\ncall_atom received" << endl;
			},
			//[=](down_atom) {
			//	self->send(self->state.lift, down_atom::value);
			//},
			//[=](result_atom, string result) {
			//	aout(self) << "\ncontroller, result received: " << result << endl;
			//},
			[&](const exit_msg& ex) {
				self->quit();
			}
		};

	}

	void start_controller(actor_system& system, const elevator::config& cfg) {
		auto controller = system.spawn(controller_actor);
		// try to publish math actor at given port
		cout << "*** try publish at port " << cfg.port << endl;
		auto expected_port = io::publish(controller, cfg.port);
		if (!expected_port) {
			std::cerr << "*** publish failed: "
				<< system.render(expected_port.error()) << endl;
			return;
		}
		cout << "*** lift controller successfully published at port " << *expected_port << endl
			<< "*** press [enter] to quit" << endl;
		string dummy;
		std::getline(std::cin, dummy);
		cout << "... cya" << endl;
		anon_send_exit(controller, exit_reason::user_shutdown);
	}
}

