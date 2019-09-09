#pragma once

#include <queue>
#include <vector>
#include <set>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator_actor.hpp"
#include "elevator/schedule.hpp"

namespace dispatcher
{
	enum class direction
	{
		up,
		down,
		nowhere,
	};

	class journey
	{
	public:

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

	using journey_queue_t = std::queue<std::unique_ptr<journey>>;
	using journey_queue_list_t = std::vector<journey_queue_t>;

	typedef std::queue<strong_actor_ptr> floor_pickup_wait_queue_t;
	typedef std::vector<strong_actor_ptr> actor_list_t;


	//using down_elevator_schedule = std::map<int, std::unique_ptr<elevator_schedule_item>>;

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


	class dispatcher_actor : public event_based_actor
	{
	public:

		dispatcher_actor(actor_config& cfg, const actor& controller_actor);

		behavior make_behavior() override;

	protected:

		const actor& controller_actor_;

		void add_subscriber(const strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type);
		std::map<std::string, strong_actor_ptr> debug_message_subscribers;

		void debug_msg(std::string msg);

		//std::queue<std::unique_ptr<journey>> undispached_journeys;
		//std::queue<strong_actor_ptr> idle_elevators;

		//std::set<int, std::vector<int>, std::less<int>> down_waypoints;
		//std::set<int, std::vector<int>, std::greater<int>> up_waypoints;

		int register_elevator(const strong_actor_ptr& elevator);
		std::vector<elevator_status> elevator_statuses;

		int register_passenger(const strong_actor_ptr& passenger);
		std::vector<strong_actor_ptr> passengers;

		void schedule_journey(std::unique_ptr<journey> journey);
		void dispatch_idle_elevators();

		void notify_passengers(int elevator_number, int floor_number);

		//journey_queue_list_t up_journey_queues; 
		std::deque<schedule::elevator_schedule<strong_actor_ptr, schedule::UP>> up_schedules;

		//journey_queue_list_t down_journey_queues;
		std::deque<schedule::elevator_schedule<strong_actor_ptr, schedule::DOWN>> down_schedules;

	};

}



