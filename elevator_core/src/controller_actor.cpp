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
			[=](register_elevator_atom) {
				aout(self) << "\nregister_elevator_atom received" << endl;
				auto elevator = self->current_sender();
				self->state.elevator = elevator;
				self->monitor(elevator);
			},
			[=](register_passenger_atom) {
				auto passenger = self->current_sender();
				aout(self) << "\nregister_passenger_atom received" << endl;
				self->state.passenger = passenger;
				self->monitor(passenger);
			},
			[=](call_atom, int from_floor, int to_floor) {
				aout(self) << "\ncontroller: call_atom received, from_floor: " << from_floor << ", to_floor: " << to_floor << endl;
				
				auto passenger = self->current_sender();
				self->state.passenger_from_floor = from_floor;
				self->state.passenger_to_floor = to_floor;
				if (self->state.elevator)
				{
					self->send(self->state.elevator, elevator::waypoint_atom::value, from_floor);
					self->send(self->state.elevator, elevator::waypoint_atom::value, to_floor);
				}
				
			},
			[=](waypoint_arrived_atom, int floor)
			{

				aout(self) << "\ncontroller: waypoint_arrived_atom received, floor: " << floor << endl;
				if(self->state.passenger)
				{
					if(floor == self->state.passenger_from_floor)
						self->send(self->state.passenger, elevator::elevator_arrived_atom::value);
					else if(floor == self->state.passenger_to_floor)
						self->send(self->state.passenger, elevator::destination_arrived_atom::value, floor);
				}

			},
			[&](const exit_msg& ex) {
				self->quit();
			}
		};

	}
}

