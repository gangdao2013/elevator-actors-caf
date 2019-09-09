
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


		//for (int i = 0; i < MAX_FLOORS; i++)
		//{
		//	up_journey_queues[i] = journey_queue_t{};
		//	down_journey_queues[i] = journey_queue_t{};
		//}

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

		if (journey->direction == direction::up)
		{
			bool inserted_in_existing_schedule = false;
			if (up_schedules.size() > 0)
			{
				for (auto itr = up_schedules.begin(); itr != up_schedules.end(); itr++)
				{
					if (itr->has_capacity(journey->from_floor, journey->to_floor, 1))
					{
						itr->insert_journey(journey->passenger, journey->from_floor, journey->to_floor);
						inserted_in_existing_schedule = true;
					}
				}
			}
			if (!inserted_in_existing_schedule)
			{
				// create a new schedule and add to end of up schedule list
				schedule::elevator_schedule<strong_actor_ptr, schedule::UP> schedule;
				schedule.insert_journey(journey->passenger, journey->from_floor, journey->to_floor);
				up_schedules.push_back(std::move(schedule));
			}
		} 

		if (journey->direction == direction::down)
		{
			bool inserted_in_existing_schedule = false;
			if (down_schedules.size() > 0)
			{
				for (auto itr = down_schedules.begin(); itr != down_schedules.end(); itr++)
				{
					if (itr->has_capacity(journey->from_floor, journey->to_floor, 1))
					{
						itr->insert_journey(journey->passenger, journey->from_floor, journey->to_floor);
						inserted_in_existing_schedule = true;
					}
				}
			}
			if (!inserted_in_existing_schedule)
			{
				// create a new schedule and add to end of up schedule list
				schedule::elevator_schedule<strong_actor_ptr, schedule::DOWN> schedule;
				schedule.insert_journey(journey->passenger, journey->from_floor, journey->to_floor);
				down_schedules.push_back(std::move(schedule));
			}
		}
	}

	void dispatcher_actor::dispatch_idle_elevators()
	{

		// Down journeys first
		if (down_schedules.size() > 1)
		{
			// check for idle elevator
			int size = elevator_statuses.size();
			for (int i = 0; i < size; i++)
			{
				if (elevator_statuses[i].idle)
				{
					elevator_statuses[i].idle = false;
					elevator_statuses[i].motion = elevator_motion::moving_down;
					elevator_statuses[i].waypoints.clear();

					// set up and dispatch the waypoints for the elevator
					auto waypoints = std::move(down_schedules.front().get_waypoints_queue());
					while (waypoints.size() > 0)
					{
						send(elevator_statuses[i].elevator, waypoint_atom::value, waypoints.front()->floor);
						elevator_statuses[i].waypoints[waypoints.front()->floor] = std::move(waypoints.front());
						waypoints.pop();
					}
					down_schedules.pop_front();
				}					
			}
		}

		// Up journey next
		if (up_schedules.size() > 1)
		{
			// check for idle elevator
			int size = elevator_statuses.size();
			for (int i = 0; i < size; i++)
			{
				if (elevator_statuses[i].idle)
				{
					elevator_statuses[i].idle = false;
					elevator_statuses[i].motion = elevator_motion::moving_up;
					elevator_statuses[i].waypoints.clear();

					// set up and dispatch the waypoints for the elevator
					auto waypoints = std::move(up_schedules.front().get_waypoints_queue());
					while (waypoints.size() > 0)
					{
						send(elevator_statuses[i].elevator, waypoint_atom::value, waypoints.front()->floor);
						elevator_statuses[i].waypoints[waypoints.front()->floor] = std::move(waypoints.front());
						waypoints.pop();
					}
					up_schedules.pop_front();
				}
			}
		}
	}

	void dispatcher_actor::notify_passengers(int elevator_number, int floor_number)
	{
		// drop offs
		std::vector<strong_actor_ptr>& dropoffs = elevator_statuses[elevator_number].waypoints[floor_number]->dropoff_list;
		for (auto passenger : dropoffs)
		{
			send(passenger, disembark_atom::value, floor_number);
		}

		std::vector<strong_actor_ptr>& pickups = elevator_statuses[elevator_number].waypoints[floor_number]->pickup_list;
		for (auto passenger : pickups)
		{
			send(passenger, embark_atom::value, elevator_number);
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

		elevator_status status(elevator);
//		status.elevator = std::move(elevator);


		elevator_statuses.push_back(std::move(status));
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