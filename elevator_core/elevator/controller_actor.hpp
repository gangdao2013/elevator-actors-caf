#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/elevator.hpp"


using namespace caf;

namespace controller 
{

	struct passenger_journey
	{
		strong_actor_ptr passenger;
		int from_floor;
		int to_floor;
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


	struct controller_state
	{
		strong_actor_ptr scheduler;

		strong_actor_ptr elevator;

	};

	behavior scheduler_actor(stateful_actor<scheduler_state>* self, actor & controller);

	behavior controller_actor(stateful_actor<controller_state>* self);
}