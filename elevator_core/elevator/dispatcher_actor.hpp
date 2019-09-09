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
	//using up_journey_queues = std::map<int, passenger_journey_queue, std::greater<int>>;
	//using down_journey_queues = std::map<int, passenger_journey_queue, std::less<int>>;


	typedef std::queue<strong_actor_ptr> floor_pickup_wait_queue_t;
	typedef std::vector<strong_actor_ptr> actor_list_t;


	//using down_elevator_schedule = std::map<int, std::unique_ptr<elevator_schedule_item>>;

	struct elevator_status
	{
		strong_actor_ptr elevator;
		bool idle;
		elevator::elevator_motion motion;
		int current_floor;
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

		std::queue<std::unique_ptr<journey>> undispached_journeys;
		std::queue<strong_actor_ptr> idle_elevators;

		//std::set<int, std::vector<int>, std::less<int>> down_waypoints;
		//std::set<int, std::vector<int>, std::greater<int>> up_waypoints;

		int register_elevator(const strong_actor_ptr& elevator);
		std::vector<elevator_status> elevator_statuses;

		int register_passenger(const strong_actor_ptr& passenger);
		std::vector<strong_actor_ptr> passengers;

		void schedule_journey(std::unique_ptr<journey> journey);
		void dispatch_idle_elevators();

		void notify_passengers(int elevator_number, int floor_number);

		journey_queue_list_t up_journey_queues; 
		schedule::elevator_schedule<std::greater<int>> up_schedule;;

		journey_queue_list_t down_journey_queues;
		schedule::elevator_schedule<std::less<int>> down_schedule;

	};

}



