#include <vector>
#include <queue>
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


	vector<queue<passenger_journey>> pick_ups(elevator::FLOOR_MAX);
	vector<vector<passenger_journey>> drop_offs(elevator::FLOOR_MAX);

	void schedule_journey(stateful_actor<scheduler_state>* self, std::unique_ptr<passenger_journey> journey)
	{
		auto state = self->state;

		state.pickup_list[journey->from_floor].emplace(journey->passenger);
		state.dropoff_list[journey->to_floor].emplace_back(journey->passenger);
		state.waypoints.emplace(journey->from_floor);
		state.waypoints.emplace(journey->to_floor);

	}

	behavior scheduler_actor(stateful_actor<scheduler_state>* self, actor & controller)
	{
		auto state = self->state;
		state.controller = actor_cast<strong_actor_ptr>(controller);

		for (int floor = 0; floor < elevator::FLOOR_MAX; floor++)
		{
			state.pickup_list.emplace_back(actor_queue_t());
			state.dropoff_list.emplace_back(actor_list_t());
		}

		return
		{
			[=](call_atom, strong_actor_ptr passenger, int from_floor, int to_floor)
			{
				aout(self) << "\scheduler: call_atom received, from_floor: " << from_floor << ", to_floor: " << to_floor << endl;
				auto journey = std::make_unique<passenger_journey>(passenger, from_floor, to_floor);
				schedule_journey(self, std::move(journey));
			},
			[=](request_elevator_schedule_atom, int elevator_number)
			{
				aout(self) << "\scheduler: request_elevator_schedule_atom received, for elevator: " << elevator_number << endl;
			},
			[=](elevator_idle_atom, int elevator_number)
			{
				aout(self) << "\scheduler: elevator_idle_atom received, for elevator: " << elevator_number << endl;
			},
			[=](waypoint_arrived_atom, int elevator_number)
			{
				aout(self) << "\scheduler: waypoint_arrived_atom received, for elevator: " << elevator_number << endl;
			}
		};

	}

	behavior controller_actor(stateful_actor<controller_state>* self)
	{
		// set up data structures

		auto state = self->state;

		auto scheduler = self->spawn(scheduler_actor, self);
		self->monitor(scheduler);
		state.scheduler = actor_cast<strong_actor_ptr>(scheduler);

		self->set_down_handler([=](const down_msg& dm)
		{
			if (dm.source == self->state.scheduler)
			{
				aout(self) << "\controller: losing connection to scheduler, respawning" << endl;
				auto scheduler = self->spawn(scheduler_actor, self);
				self->monitor(scheduler);
				self->state.scheduler = actor_cast<strong_actor_ptr>(scheduler);
			}
		});

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
				//self->state.passenger = passenger;
				self->monitor(passenger);
			},
			[=](call_atom, int from_floor, int to_floor) {
				aout(self) << "\ncontroller: call_atom received, from_floor: " << from_floor << ", to_floor: " << to_floor << endl;
				
				auto passenger = self->current_sender();
				self->send(self->state.scheduler, call_atom::value, passenger, from_floor, to_floor);

				//self->state.passenger_from_floor = from_floor;
				//self->state.passenger_to_floor = to_floor;
				//if (self->state.elevator)
				//{
				//	self->send(self->state.elevator, elevator::waypoint_atom::value, from_floor);
				//	self->send(self->state.elevator, elevator::waypoint_atom::value, to_floor);
				//}
				
			},
			[=](elevator_idle_atom, int elevator_number)
			{
				self->send(scheduler, request_elevator_schedule_atom::value, elevator_number);
			},
			[=](waypoint_arrived_atom, int floor)
			{

				//aout(self) << "\ncontroller: waypoint_arrived_atom received, floor: " << floor << endl;
				//if(self->state.passenger)
				//{
				//	if(floor == self->state.passenger_from_floor)
				//		self->send(self->state.passenger, elevator::elevator_arrived_atom::value);
				//	else if(floor == self->state.passenger_to_floor)
				//		self->send(self->state.passenger, elevator::destination_arrived_atom::value, floor);
				//}

			},
			[&](const exit_msg& ex) {
				self->quit();
			}
		};

	}
}

