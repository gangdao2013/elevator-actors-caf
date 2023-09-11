
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
	// dispatcher actor

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

	}

	// Overridden make_behaviour function - required for actor classes
	// What is returned is an initialiser list of message handling lambdas that are used as the input
	// to a behavior constructor.
	// CAF matches incoming messages against the types of the handler lambda functions.
	
	behavior dispatcher_actor::make_behavior()
	{

		// Handlers for messages coming from controller, passengers and elevators. Also directly from controller_repl

		return
		{
			[=](call_atom, int from_floor, int to_floor)
			{
				// passenger calls for an elevator
				debug_msg ("dispatcher: call_atom received, from_floor: " +  std::to_string(from_floor) + ", to_floor: " + std::to_string(to_floor));
				auto passenger = current_sender();
				auto journey_ = std::make_unique<journey>(passenger, from_floor, to_floor);
				schedule_journey(std::move(journey_));
				timer_pulse(5); // delay doing anything else to give other passengers a chance to call for an elevator
			},
			[=](dispatch_idle_atom)
			{
				dispatch_idle_elevators();
			},
			[=](timer_atom)
			{
				dispatch_idle_elevators();
				timer_guard = false;
			},
			[=](elevator_idle_atom, int elevator_number, int floor)
			{
				// elevator is idle, reset its status, then set off a dispatch timer pulse
				debug_msg("dispatcher: elevator_idle_atom received, for elevator: " + std::to_string(elevator_number));
				elevator_statuses[elevator_number].idle = true;
				elevator_statuses[elevator_number].motion = elevator_motion::stationary;
				elevator_statuses[elevator_number].current_floor = floor;
				elevator_statuses[elevator_number].waypoints.clear();
				timer_pulse(5);
			},
			[=](waypoint_arrived_atom, int elevator_number, int floor_number)
			{
				// elevator arrived at a waypoint, let relevant passengers know
				debug_msg("dispatcher: waypoint_arrived_atom received, for elevator: " + std::to_string(elevator_number) + " for floor: " + std::to_string(floor_number));
				notify_passengers(elevator_number, floor_number);
			},
			[=](register_elevator_atom, strong_actor_ptr elevator)
			{
				// register this elevator, let it know it's number and dispatcher (this)
				debug_msg("register_elevator_atom received");
				//auto elevator = current_sender();
				int elevator_number = register_elevator(elevator);
				send(actor_cast<caf::actor>(elevator), register_dispatcher_atom_v);
				send(actor_cast<caf::actor>(elevator), set_number_atom_v, elevator_number);
			},
			[=](register_passenger_atom, strong_actor_ptr passenger)
			{
				// register this passenter, let it know it's dispatcher (this)
				debug_msg("register_passenger_atom received");
				//auto passenger = current_sender();
				register_passenger(passenger);
				send(actor_cast<caf::actor>(passenger), register_dispatcher_atom_v);
			},
			[=](get_current_state_name_atom)
			{
				return "running";
			},
			[=](get_name_atom)
			{
				return "dispatcher";
			},
			[=](elevator_subscribe_atom, std::string subscriber_key, elevator_observable_event_type event_type)
			{
				// subscribe to dispatcher events too
				add_subscriber(current_sender(), subscriber_key, event_type);
				debug_msg("subscribe_atom received");

			},
			[=](elevator_subscribe_atom, strong_actor_ptr subscriber, std::string subscriber_key, elevator_observable_event_type event_type)
			{
				// subscribe to dispatcher events too
				add_subscriber(subscriber, subscriber_key, event_type);
				debug_msg("subscribe_atom received");

			},
		};
	}

	// send a timer pulse - mainly used to delay dispatching schedules
	void dispatcher_actor::timer_pulse(int seconds)
	{
		if(!timer_guard)
			delayed_send(this, std::chrono::seconds(seconds), timer_atom_v);
	}

	// schedule this journey, either in an existing schedule or a new one if there are none or none with capacity
	void dispatcher_actor::schedule_journey(std::unique_ptr<journey> journey)
	{

		// most of the heavy lifting is in the schedule class, it 

		if (journey->direction == direction::up)
		{
			bool inserted_in_existing_schedule = false;
			if (up_schedules.size() > 0) // i.e. are there any existing schedules we could check?
			{
				for (auto itr = up_schedules.begin(); itr != up_schedules.end(); itr++)
				{
					if (itr->has_capacity(journey->from_floor, journey->to_floor, 1))
					{
						// this schedule has capacity, so we'll use it
						itr->insert_journey(journey->passenger, journey->from_floor, journey->to_floor);
						inserted_in_existing_schedule = true;
						break;
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
			if (down_schedules.size() > 0) // are there any schedules we could check?
			{
				for (auto itr = down_schedules.begin(); itr != down_schedules.end(); itr++)
				{
					if (itr->has_capacity(journey->from_floor, journey->to_floor, 1))
					{
						// this schedule has capacity, so we'll use it
						itr->insert_journey(journey->passenger, journey->from_floor, journey->to_floor);
						inserted_in_existing_schedule = true;
						break;
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

	// check for idle elevators, and dispatch any schedules to them
	void dispatcher_actor::dispatch_idle_elevators()
	{
		// Note: this dispatch algorithm just preferences down schedules over up schedules.
		// A refinement would be to balance the demand for up/down more evenly, or according to time of day, load, etc.
		// Next release!

		// Down journeys first
		if (down_schedules.size() > 0)
		{
			// check for idle elevator
			int size = elevator_statuses.size();
			for (int i = 0; i < size && down_schedules.size() > 0; i++)
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
						send(actor_cast<caf::actor>(elevator_statuses[i].elevator), waypoint_atom_v, waypoints.front()->floor);
						elevator_statuses[i].waypoints[waypoints.front()->floor] = std::move(waypoints.front());
						waypoints.pop();
					}
					down_schedules.pop_front();
				}					
			}
		}

		// Up journey next
		if (up_schedules.size() > 0)
		{
			// check for idle elevator
			int size = elevator_statuses.size();
			for (int i = 0; i < size && up_schedules.size() > 0; i++)
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
						send(actor_cast<caf::actor>(elevator_statuses[i].elevator), waypoint_atom_v, waypoints.front()->floor);
						elevator_statuses[i].waypoints[waypoints.front()->floor] = std::move(waypoints.front());
						waypoints.pop();
					}
					up_schedules.pop_front();
				}
			}
		}
	}

	// Notify pickups/dropoffs when their elevator arrives - done via a message to the relevant passenger actors
	void dispatcher_actor::notify_passengers(int elevator_number, int floor_number)
	{
		// drop offs
		std::vector<strong_actor_ptr>& dropoffs = elevator_statuses[elevator_number].waypoints[floor_number]->dropoff_list;
		int s = dropoffs.size();
		for (auto passenger : dropoffs)
		{
			send(actor_cast<caf::actor>(passenger), disembark_atom_v, floor_number);
		}

		// pickups
		std::vector<strong_actor_ptr>& pickups = elevator_statuses[elevator_number].waypoints[floor_number]->pickup_list;
		for (auto passenger : pickups)
		{
			send(actor_cast<caf::actor>(passenger), embark_atom_v, elevator_number);
		}

	}

	// elevator registering itself, add it to the list of available elevators
	int dispatcher_actor::register_elevator(const strong_actor_ptr& elevator)
	{
		// check actor not registered already, otherwise add to list and return elevator number (= index)

		int size = elevator_statuses.size();
		for (int i = 0; i < size; i++)
		{
			if (elevator_statuses[i].elevator == elevator)
				return i;
		}

		elevator_status status(elevator); // NB: move constructor

		elevator_statuses.push_back(std::move(status));
		monitor(elevator);
		return size;
	}

	// passenger registering itself - not really used for now, but there for future enhancement
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

	// send debug message to subscriber actors, e.g. repl
	void dispatcher_actor::debug_msg(std::string msg)
	{
		std::string subscriber_msg = "[dispatcher]: " + msg;
		for (auto kv : debug_message_subscribers)
		{
			auto recipient = actor_cast<actor>(kv.second);
			send(recipient, message_atom_v, subscriber_msg);
		}

	}

	// register debug message subscribers, e.g. repl
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