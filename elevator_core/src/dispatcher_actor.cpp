
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/string_util.hpp"
#include "elevator/elevator.hpp"

#include "elevator/dispatcher_actor.hpp"
#include "elevator/schedule.hpp"


using namespace caf;
using namespace elevator;

namespace dispatcher
{

	//vector<queue<passenger_journey>> pick_ups(elevator::FLOOR_MAX);
	//vector<vector<passenger_journey>> drop_offs(elevator::FLOOR_MAX);

	//void schedule_journey(stateful_actor<scheduler_state>* self, std::unique_ptr<passenger_journey> journey)
	//{
	//	auto state = self->state;

	//	state.pickup_list[journey->from_floor].emplace(journey->passenger);
	//	state.dropoff_list[journey->to_floor].emplace_back(journey->passenger);
	//	state.waypoints.emplace(journey->from_floor);
	//	state.waypoints.emplace(journey->to_floor);

	//}

	



	dispatcher_actor::dispatcher_actor(actor_config& cfg, const actor& controller_actor) : event_based_actor(cfg)
		, controller_actor_{ std::move(controller_actor) }
	{

		//set_default_handler([=](scheduled_actor* actor, message_view& view)
		//	{
		//		aout(this) << "dispatcher_actor: unknown message: " << view << std::endl;
		//		//return caf::message{};
		//		return sec::unexpected_message;
		//	});

		set_error_handler([=](scheduled_actor* actor, error& err) -> void
			{
				aout(this) << "dispatcher_actor: error: " << err << std::endl;
			});


		for (int i = 0; i < MAX_FLOORS; i++)
		{
			up_journey_queues[i] = journey_queue_t{};
			down_journey_queues[i] = journey_queue_t{};

			//up_schedule.push_back(std::move(std::make_unique<elevator_schedule_item>()));
			//up_schedule[i]->floor = i;
			//up_schedule[i]->unused_capacity = 0;
			//down_schedule.push_back(std::move(std::make_unique<elevator_schedule_item>()));
			//down_schedule[i]->floor = i;
			//down_schedule[i]->unused_capacity = 0;
		}

	}

	behavior dispatcher_actor::make_behavior()
	{

		return
		{
			[=](call_atom, int from_floor, int to_floor)
			{
				debug_msg ("dispatcher: call_atom received, from_floor: " +  std::to_string(from_floor) + ", to_floor: " + std::to_string(to_floor));
				auto passenger = current_sender();
				auto journey_ = std::make_unique<journey>(passenger, from_floor, to_floor);
				schedule_journey(std::move(journey_));
				dispatch_idle_elevators();
			},
			[=](request_elevator_schedule_atom, int elevator_number)
			{
				debug_msg("dispatcher: request_elevator_schedule_atom received, for elevator: " + std::to_string(elevator_number));
			},
			[=](elevator_idle_atom, int elevator_number, int floor)
			{
				debug_msg("dispatcher: elevator_idle_atom received, for elevator: " + std::to_string(elevator_number));
				elevator_statuses[elevator_number].idle = true;
				elevator_statuses[elevator_number].motion = elevator_motion::stationary;
				elevator_statuses[elevator_number].current_floor = floor;
				dispatch_idle_elevators();
			},
			[=](waypoint_arrived_atom, int elevator_number, int floor_number)
			{
				debug_msg("dispatcher: waypoint_arrived_atom received, for elevator: " + std::to_string(elevator_number) + " for floor: " + std::to_string(floor_number));
				notify_passengers(elevator_number, floor_number);
			},
			[=](register_elevator_atom, strong_actor_ptr elevator)
			{
				debug_msg("register_elevator_atom received");
				//auto elevator = current_sender();
				int elevator_number = register_elevator(elevator);
				send(elevator, register_dispatcher_atom::value);
				send(elevator, set_number_atom::value, elevator_number);
			},
			[=](register_passenger_atom, strong_actor_ptr passenger)
			{
				debug_msg("register_passenger_atom received");
				//auto passenger = current_sender();
				register_passenger(passenger);
				send(passenger, register_dispatcher_atom::value);
			},
			[=](get_current_state_name_atom)
			{
				return "running";
			},
			[=](get_name_atom)
			{
				return "dispatcher";
			},
			[=](elevator::subscribe_atom sub, std::string subscriber_key, elevator_observable_event_type event_type)
			{
				// subscribe to dispatcher events too
				add_subscriber(current_sender(), subscriber_key, event_type);
				debug_msg("subscribe_atom received");

			},
			[=](elevator::subscribe_atom sub, strong_actor_ptr subscriber, std::string subscriber_key, elevator_observable_event_type event_type)
			{
				// subscribe to dispatcher events too
				add_subscriber(subscriber, subscriber_key, event_type);
				debug_msg("subscribe_atom received");

			},
		};
	}

	void dispatcher_actor::schedule_journey(std::unique_ptr<journey> journey)
	{
		// TODO: replace this with directional priority queues, with multiple waypoints

		/*
		
		up scheduling pseudo code
		
		start_floor = min(up_journey_queues index)

		elevator_capacity(start_floor) = max_elevator_capacity

		for( i = start_floor; i <= max_floor; i++)

			for (q = 0 to elevator_capacity(i))
				journey = up_journey_queue(i).pop()
				pickup_list(i) += journey.passenger
				drop_off_list(journey.to_floor) += journey.passenger		
			elevator_capacity(i+1) = elevator_capacity(i) - len(dropoff_list(i+1)
		
		*/

		//if (journey->direction == journey_direction::down)
		//	down_journey_queues[journey->from_floor].push(std::move(journey));
		//else if (journey->direction == journey_direction::up)
		//	up_journey_queues[journey->from_floor].push(std::move(journey));
		//else
		//	return;

		//// reset capacities
		//for (int i = 0; i < MAX_FLOORS; i++)
		//	up_schedule[i]->unused_capacity = ELEVATOR_CAPACITY_MAX;

		//if (up_journey_queues.size() > 0)
		//{
		//	up_schedule[0]->unused_capacity = ELEVATOR_CAPACITY_MAX;
		//	int max_floor = up_journey_queues.size();
		//	for (int current_floor = 0; current_floor < max_floor; current_floor++)
		//	{
		//		journey_queue_t & waiting_journey_queue = up_journey_queues[current_floor]; //note use of ref here, otherwise we get copy assignment of queue of unique vectors which is invalid;

		//		while 
		//		(
		//			(waiting_journey_queue.size() > 0) 
		//			&& (up_schedule[current_floor]->unused_capacity != 0)
		//		)
		//		{
		//			auto passenger = waiting_journey_queue.front()->passenger;
		//			int to_floor = waiting_journey_queue.front()->to_floor;

		//			up_schedule[current_floor]->pickup_list.push_back(passenger);
		//			up_schedule[current_floor]->unused_capacity--;

		//			up_schedule[to_floor]->dropoff_list.push_back(std::move(waiting_journey_queue.front())); // schedule this journey
		//			
		//			waiting_journey_queue.pop();

		//			if (current_floor < max_floor)
		//			{
		//				int next_floor = current_floor + 1;
		//				up_schedule[next_floor]->unused_capacity 
		//					= up_schedule[current_floor]->unused_capacity + up_schedule[next_floor]->dropoff_list.size();
		//			}
		//		}
		//	}
		//}

		/*
		down scheduling pseudo code

		start_floor = max(down_journey_queue index)

		elevator_capacity(start_floor) = max_elevator_capacity
		
		for( i = start_floor; i >= 0; i--)

			for (q = 0 to elevator_capacity(i))
				journey = down_journey_queue(i).pop()
				pickup_list(i) += journey.passenger
				drop_off_list(journey.to_floor) += journey.passenger		
			elevator_capacity(i-1) = elevator_capacity(i) - len(dropoff_list(i-1)
		
		*/

		// reset capacities
		// reset capacities
		//for (int i = 0; i < MAX_FLOORS; i++)
		//	down_schedule[i]->unused_capacity = ELEVATOR_CAPACITY_MAX;

		//if (down_journey_queues.size() > 0)
		//{
		//	down_schedule[MAX_FLOORS]->unused_capacity = ELEVATOR_CAPACITY_MAX;
		//	int max_floor = up_journey_queues.size();
		//	for (int current_floor = MAX_FLOORS; current_floor > 0; current_floor--)
		//	{
		//		journey_queue_t& waiting_journey_queue = down_journey_queues[current_floor]; //note use of ref here, otherwise we get copy assignment of queue of unique vectors which is invalid;

		//		while
		//			(
		//			(waiting_journey_queue.size() > 0)
		//				&& (down_schedule[current_floor]->unused_capacity != 0)
		//				)
		//		{
		//			auto passenger = waiting_journey_queue.front()->passenger;
		//			int to_floor = waiting_journey_queue.front()->to_floor;

		//			down_schedule[current_floor]->pickup_list.push_back(passenger);
		//			down_schedule[current_floor]->unused_capacity--;

		//			down_schedule[to_floor]->dropoff_list.push_back(std::move(waiting_journey_queue.front())); // schedule this journey

		//			waiting_journey_queue.pop();

		//			if (current_floor > 0)
		//			{
		//				int next_floor = current_floor - 1;
		//				down_schedule[next_floor]->unused_capacity
		//					= down_schedule[current_floor]->unused_capacity + down_schedule[next_floor]->dropoff_list.size();
		//			}
		//		}
		//	}
		//}



		// for now, just to get it going, first in best dressed
		//undispached_journeys.push(std::move(journey));
		//if (journey->from_floor == journey->to_floor)
		//	return;
		//if (journey->from_floor > journey->to_floor) // down trip
		//{
		//	down_waypoints.insert(journey->from_floor);
		//	down_waypoints.insert(journey->to_floor);
		//}
		//else
		//{
		//	up_waypoints.insert(journey->from_floor);
		//	up_waypoints.insert(journey->to_floor);
		//}

	}

	void dispatcher_actor::dispatch_idle_elevators()
	{
		// TODO: replace this with directional priority queues, with multiple waypoints

		//// Down journeys first
		//if (down_waypoints.size() > 1)
		//{
		//	// check for idle elevator
		//	int size = elevator_statuses.size();
		//	for (int i = 0; i < size; i++)
		//	{
		//		if (elevator_statuses[i].idle)
		//		{
		//			elevator_statuses[i].idle = false;
		//			elevator_statuses[i].motion = elevator_motion::moving_up;

		//			int count = 0;
		//			while(down_waypoints.size() != 0 && count < 4) // don't send more than 4 jobs 

		//			int waypoint = *(down_waypoints.begin());
		//			send(elevator_statuses[i].elevator, waypoint_atom::value, waypoint);
		//			down_waypoints.erase(waypoint);
		//			if (down_waypoints.size > 0)
		//			{
		//				waypoint = *(down_waypoints.begin());
		//				down_waypoints.erase()
		//			send(elevator_statuses[i].elevator, waypoint_atom::value, undispached_journeys.front()->to_floor);
		//			}

		//			break;
		//		}					
		//	}
		//}
	}

	void dispatcher_actor::notify_passengers(int elevator_number, int floor_number)
	{
		// pick up

		if (undispached_journeys.size() > 0)
		{
			if (floor_number == undispached_journeys.front()->from_floor)
			{
				auto passenger = undispached_journeys.front()->passenger;
				send(passenger, embark_atom::value);
			}

			// drop offs
			if (floor_number == undispached_journeys.front()->to_floor)
			{
				auto passenger = undispached_journeys.front()->passenger;
				send(passenger, disembark_atom::value, floor_number);

				//send(journeys.front()->passenger, disembark_atom::value, floor_number);
				undispached_journeys.pop(); // this journey is over
			}
		}
	}

	int dispatcher_actor::register_elevator(const strong_actor_ptr& elevator)
	{
		// check actor not registered already, otherwise add to list and return elevator number (= index)

		int size = elevator_statuses.size();
		for (int i = 0; i < size; i++)
		{
			if (elevator_statuses[i].elevator == elevator)
				return i;
		}

		elevator_status status;
		status.elevator = std::move(elevator);
		status.idle = true;

		elevator_statuses.emplace_back(status);
		monitor(elevator);
		return size;
	}

	int dispatcher_actor::register_passenger(const strong_actor_ptr& passenger)
	{
		// check actor not registered already, otherwise add to list and return elevator number (= index + 1)

		int size = passengers.size();
		for (int i = 0; i < size; i++)
		{
			if (passengers[i] == passenger)
				return i + 1;
		}
		passengers.emplace_back(std::move(passenger));
		monitor(passenger);
		return size;
	}

	void dispatcher_actor::debug_msg(std::string msg)
	{
		std::string subscriber_msg = "[dispatcher]: " + msg;
		for (auto kv : debug_message_subscribers)
		{
			auto recipient = actor_cast<actor>(kv.second);
			send(recipient, message_atom::value, subscriber_msg);
		}

	}

	void dispatcher_actor::add_subscriber(strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type)
	{
		// add subscriber to relevant subscriber map
		switch (event_type)
		{
		case elevator_observable_event_type::debug_message:
		{
			// nb: deliberately replace if key is the same, need to destroy any existing ref
			auto existing_ptr = debug_message_subscribers[subscriber_key];
			if (existing_ptr)
			{
				auto handle = actor_cast<actor>(existing_ptr);
				destroy(handle);
			}
			debug_message_subscribers[subscriber_key] = std::move(subscriber);
		}
		break;
		default:
			break;
		};
	}
}