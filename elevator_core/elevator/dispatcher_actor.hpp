#pragma once

#include <queue>
#include <vector>
#include <set>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

namespace dispatcher
{

	struct passenger_journey
	{
		strong_actor_ptr passenger;
		int from_floor;
		int to_floor;

		passenger_journey(const strong_actor_ptr passenger, int from_floor, int to_floor ):
			passenger{passenger}
			, from_floor{from_floor}
			, to_floor{ to_floor }{}

		//bool operator < (passenger_journey const& other)const { return from_floor < other.from_floor; }
	};

	typedef std::queue<strong_actor_ptr> actor_queue_t;
	typedef std::vector<strong_actor_ptr> actor_list_t;
	typedef std::vector<actor_queue_t> floor_pickup_list_t;
	typedef std::vector<actor_list_t> floor_dropoff_list_t;

	struct scheduler_state
	{
		strong_actor_ptr controller;
		floor_pickup_list_t pickup_list;
		floor_dropoff_list_t dropoff_list;
		std::queue<int> waypoints;
	};

	struct elevator_status
	{
		strong_actor_ptr elevator;
		bool idle;
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

		std::queue<std::shared_ptr<passenger_journey>> journeys;
		std::queue<strong_actor_ptr> idle_elevators;


		int register_elevator(const strong_actor_ptr& elevator);
		std::vector<elevator_status> elevator_statuses;

		int register_passenger(const strong_actor_ptr& passenger);
		std::vector<strong_actor_ptr> passengers;

		void schedule_journey(std::shared_ptr<passenger_journey> journey);
		void dispatch_next_journey();

		void notify_passengers(int elevator_number, int floor_number);

	};

}



