#pragma once

#include <queue>
#include <vector>
#include <set>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator_actor.hpp"
#include "elevator/schedule.hpp"

/*

Namespace dispatcher contains the core data structures and algorithms for scheduling and dispatching passenger journeys
amongst available elevators.

*/


namespace dispatcher
{
	enum class direction
	{
		up,
		down,
		nowhere,
	};

	// journey holds information about an unscheduled journey
	class journey
	{
	public:

		// the passenger actor, where they are going
		strong_actor_ptr passenger;
		int from_floor;
		int to_floor;
		direction direction;

		journey(const strong_actor_ptr passenger, int from_floor, int to_floor ):
			passenger{passenger}
			, from_floor{from_floor}
			, to_floor{ to_floor }
		{
			if (from_floor < to_floor)
				direction = direction::up;
			else if (from_floor > to_floor)
				direction = direction::down;
			else
				direction = direction::nowhere;
		}

		//bool operator < (passenger_journey const& other)const { return from_floor < other.from_floor; }
	};


	// elevator_status contains all the dispatch/schedule control information for an elevator actor;
	// the key field is 'waypoints' - a list of floors where the elevator will stop on any given schedule
	struct elevator_status
	{
		strong_actor_ptr elevator;
		bool idle;
		elevator::elevator_motion motion;
		int current_floor;
		std::map<int, std::unique_ptr<schedule::elevator_waypoint_item<strong_actor_ptr>>> waypoints;


		elevator_status() : idle{ true }, current_floor{ 0 }, motion{ elevator::elevator_motion::stationary } {}

		elevator_status(strong_actor_ptr elevator) : elevator{ elevator }
			, idle{ true }, current_floor{ 0 }, motion{ elevator::elevator_motion::stationary }{}

		// move constructor
		elevator_status(elevator_status&& other) noexcept
		{
			elevator = std::move(other.elevator);
			other.elevator = nullptr;
			motion = other.motion;
			other.motion = elevator::elevator_motion::stationary;
			idle = other.idle;
			current_floor = other.current_floor;
			other.current_floor = 0;
			waypoints = std::move(other.waypoints);
			other.waypoints.clear();
		}

		// move assignment
		elevator_status& operator=(elevator_status&& other) noexcept
		{
			if (this == &other) return *this;

			elevator = std::move(other.elevator);
			other.elevator = nullptr;
			motion = other.motion;
			other.motion = elevator::elevator_motion::stationary;
			idle = other.idle;
			current_floor = other.current_floor;
			other.current_floor = 0;
			waypoints = std::move(other.waypoints);
			other.waypoints.clear();
			return *this;
		}
	};


	// dispatcher_actor is the main algorithmic workhorse of the system;
	// it receives floor calls from passengers, schedules the journeys based on capacities, and then dispatches schedules to elevators as waypoints

	class dispatcher_actor : public event_based_actor
	{
	public:

		dispatcher_actor(actor_config& cfg, const actor& controller_actor);

		behavior make_behavior() override;

	protected:

		const actor& controller_actor_;

		// Register subscribers for debug messages
		void add_subscriber(const strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type);
		std::map<std::string, strong_actor_ptr> debug_message_subscribers;

		// Send debug messages to all subscribers
		void debug_msg(std::string msg);

		// timer is used to delay dispatching elevators, to give time for passengers on different
		// floors to call for elevators
		bool timer_guard = false;
		void timer_pulse(int seconds);

		// Elevator actors either register themselves with a controller, or are spawned by a controller. Either way they
		// are registered with a dispatcher.
		int register_elevator(const strong_actor_ptr& elevator);
		std::vector<elevator_status> elevator_statuses;
		
		// Passenger actors either register themselves with a controller, or are spawned by a controller. Either way they
		// are registered with a dispatcher.
		int register_passenger(const strong_actor_ptr& passenger);
		std::vector<strong_actor_ptr> passengers;

		// Scheduling, dispatching and notification to passengers when it's time to embark/disembark
		void schedule_journey(std::unique_ptr<journey> journey);
		void dispatch_idle_elevators();
		void notify_passengers(int elevator_number, int floor_number);

		// Queues of up & down schedules; note the use of comparators UP/DOWN to differentiate - these are aliases for std::less & std::greater;
		// see schedule.hpp for details.
		std::deque<schedule::elevator_schedule<strong_actor_ptr, schedule::UP>> up_schedules;
		std::deque<schedule::elevator_schedule<strong_actor_ptr, schedule::DOWN>> down_schedules;

	};

}



