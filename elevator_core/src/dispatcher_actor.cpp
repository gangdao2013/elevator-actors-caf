
#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/string_util.hpp"
#include "elevator/elevator.hpp"

#include "elevator/dispatcher_actor.hpp"


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

	}



	behavior dispatcher_actor::make_behavior()
	{

		//for (int floor = 0; floor < elevator::FLOOR_MAX; floor++)
		//{
		//	state.pickup_list.emplace_back(actor_queue_t());
		//	state.dropoff_list.emplace_back(actor_list_t());
		//}

		return
		{
			[=](call_atom, int from_floor, int to_floor)
			{
				debug_msg ("dispatcher: call_atom received, from_floor: " +  std::to_string(from_floor) + ", to_floor: " + std::to_string(to_floor));
				auto passenger = current_sender();
				auto journey = std::make_shared<passenger_journey>(passenger, from_floor, to_floor);
				schedule_journey(journey);
				dispatch_next_journey();
			},
			[=](request_elevator_schedule_atom, int elevator_number)
			{
				debug_msg("dispatcher: request_elevator_schedule_atom received, for elevator: " + std::to_string(elevator_number));
			},
			[=](elevator_idle_atom, int elevator_number)
			{
				debug_msg("dispatcher: elevator_idle_atom received, for elevator: " + std::to_string(elevator_number));
				elevator_statuses[elevator_number].idle = true;
				dispatch_next_journey();
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

	void dispatcher_actor::schedule_journey(std::shared_ptr<passenger_journey> journey)
	{
		// TODO: replace this with directional priority queues, with multiple waypoints

		// for now, just to get it going, first in best dressed
		journeys.emplace(journey);
	}

	void dispatcher_actor::dispatch_next_journey()
	{
		// TODO: replace this with directional priority queues, with multiple waypoints

		if (journeys.size() > 0)
		{
			// check for idle elevator
			int size = elevator_statuses.size();
			for (int i = 0; i < size; i++)
			{
				if (elevator_statuses[i].idle)
				{
					//journeys.pop(); // pop is 

					elevator_statuses[i].idle = false;
					send(elevator_statuses[i].elevator, waypoint_atom::value, journeys.front()->from_floor);
					send(elevator_statuses[i].elevator, waypoint_atom::value, journeys.front()->to_floor);

					break;
				}					
			}
		}
	}

	void dispatcher_actor::notify_passengers(int elevator_number, int floor_number)
	{
		// pick up

		if (journeys.size() > 0)
		{
			if (floor_number == journeys.front()->from_floor)
			{
				int s = journeys.size();
				auto f = journeys.front();
				s = journeys.size();
				auto passenger = f->passenger;

				send(passenger, embark_atom::value);

			}

			// drop offs
			if (floor_number == journeys.front()->to_floor)
			{
				auto passenger = journeys.front()->passenger;
				send(passenger, disembark_atom::value, floor_number);

				//send(journeys.front()->passenger, disembark_atom::value, floor_number);
				journeys.pop(); // this journey is over
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