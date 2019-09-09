#pragma once

#include <memory>
#include <vector>
#include <map>
#include <algorithm>

#include "caf/all.hpp"

using pickup_list_t = std::vector<strong_actor_ptr>;
using dropoff_list_t = std::vector<strong_actor_ptr>;

struct elevator_schedule_item
{
	//strong_actor_ptr scheduler;
	int floor;
	int going_further_count;
	pickup_list_t pickup_list;
	dropoff_list_t dropoff_list;
	int unused_capacity() inline;
};

int elevator_schedule_item::unused_capacity() inline
{
	return ELEVATOR_CAPACITY_MAX - going_further_count - pickup_list.size() + dropoff_list.size();
}

template<class comparator>
class elevator_schedule
{
public:

	//elevator_schedule();
	elevator_schedule()
	{
		for (int i = 0; i < MAX_FLOORS; i++)
		{
			schedule[i] = std::move(std::make_unique<elevator_schedule_item>());
			schedule[i]->floor = i;
			schedule[i]->unused_capacity = ELEVATOR_CAPACITY_MAX;
		}
	}

	bool has_capacity(int from_floor, int to_floor, int passenger_numbers);
	int max_capacity(int from_floor, int to_floor);
	void insert_journey(std::unique_ptr<journey> journey);

private:

	std::map<int, std::unique_ptr<elevator_schedule_item>, comparator> schedule;
	//std::map<std::unique_ptr<elevator_schedule_item>, std::> schedule;

};




//elevator_schedule::elevator_schedule()
//{
//	for (int i = 0; i < MAX_FLOORS; i++)
//	{
//		//schedule.(std::move(std::make_unique<elevator_schedule_item>()));
//		//schedule[i]->floor = i;
//		//schedule[i]->unused_capacity = ELEVATOR_CAPACITY_MAX;
//	}
//}



template<class comparator>
bool elevator_schedule<comparator>::has_capacity(int from_floor, int to_floor, int passenger_numbers)
{
	if (from_floor == to_floor)
		return false;
	if (from_floor > to_floor)
	{
		for (int floor = from_floor; floor >= to_floor; floor--)
		{
			//if (schedule[floor]->unused_capacity() >= passenger_numbers)
			return false;
		}
	}
	else
	{
		for (int floor = from_floor; floor <= to_floor; floor++)
		{
			//if (schedule[floor]->unused_capacity() >= passenger_numbers)
			return false;
		}
	}
	return true;
}

template<class comparator>
int elevator_schedule<comparator>::max_capacity(int from_floor, int to_floor)
{
	int max_capacity = ELEVATOR_CAPACITY_MAX;

	if (from_floor == to_floor)
		return false;
	if (from_floor > to_floor)
	{
		for (int floor = from_floor; floor >= to_floor; floor--)
		{
			//if (schedule[floor]->unused_capacity() < max_capacity)
			max_capacity = schedule[floor]->unused_capacity();
		}
	}
	else
	{
		for (int floor = from_floor; floor <= to_floor; floor++)
		{
			//if (schedule[floor]->unused_capacity() < max_capacity)
			max_capacity = schedule[floor]->unused_capacity();
		}
	}
	return max_capacity;
}

template<class comparator>
void elevator_schedule<comparator>::insert_journey(std::unique_ptr<journey> journey)
{
	if (!has_capacity(journey->from_floor, journey->to_floor, 1))
		throw("no capacity");

	return;

	//while
	//	(
	//	(waiting_journey_queue.size() > 0)
	//		&& (up_schedule[current_floor]->unused_capacity != 0)
	//		)
	//{
	//	auto passenger = waiting_journey_queue.front()->passenger;
	//	int to_floor = waiting_journey_queue.front()->to_floor;

	//	up_schedule[current_floor]->pickup_list.push_back(passenger);
	//	up_schedule[current_floor]->unused_capacity--;

	//	up_schedule[to_floor]->dropoff_list.push_back(std::move(waiting_journey_queue.front())); // schedule this journey

	//	waiting_journey_queue.pop();

	//	if (current_floor < max_floor)
	//	{
	//		int next_floor = current_floor + 1;
	//		up_schedule[next_floor]->unused_capacity
	//			= up_schedule[current_floor]->unused_capacity + up_schedule[next_floor]->dropoff_list.size();
	//	}
	//}
}
