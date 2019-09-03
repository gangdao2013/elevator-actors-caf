#include <vector>
#include <string>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"
#include "elevator/controller_actor.hpp"
#include "elevator/passenger_actor.hpp"

using namespace caf;
using namespace std;

using namespace elevator;


namespace controller
{

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
			[=](call_atom, int from_floor, int to_floor) {
				//self->send(self->state.lift, up_atom::value);
				aout(self) << "\ncontroller: call_atom received, from_floor: " << from_floor << ", to_floor: " << to_floor << endl;
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
}

