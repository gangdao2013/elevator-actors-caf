#pragma once

#include <memory>
#include <vector>
#include <map>
#include <algorithm>
//#include <math.h>

#include "elevator/elevator.hpp"
#include "caf/all.hpp"

using namespace elevator;



namespace schedule
{



	using pickup_list_t = std::vector<strong_actor_ptr>;
	using dropoff_list_t = std::vector<strong_actor_ptr>;

	using DOWN = std::less<int>;
	using UP = std::greater<int>;

	enum class schedule_direction
	{
		up,
		down
	};

	struct elevator_schedule_item
	{
		//strong_actor_ptr scheduler;
		int floor;
		int going_further_count;
		pickup_list_t pickup_list;
		dropoff_list_t dropoff_list;
		inline int unused_capacity();
	};

	inline int elevator_schedule_item::unused_capacity()
	{
		return ELEVATOR_CAPACITY_MAX - going_further_count - pickup_list.size() + dropoff_list.size();
	}

	template<class direction>
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
				schedule[i]->going_further_count = 0;
				//schedule[i]->unused_capacity = ELEVATOR_CAPACITY_MAX;
			}
		}

		bool has_capacity(int from_floor, int to_floor, int passenger_numbers);
		int max_capacity(int from_floor, int to_floor);
		schedule_direction get_direction();
		//void insert_journey(std::unique_ptr<journey> journey);

	private:

		std::map<int, std::unique_ptr<elevator_schedule_item>, direction> schedule;
		//std::map<std::unique_ptr<elevator_schedule_item>, std::> schedule;

	};

	template<class direction>
	schedule_direction elevator_schedule<direction>::get_direction()
	{
		//if (&(direction::operator())
		//	== &(std::less<int>::operator()))
		//	return schedule_direction::down;
		//else
		//	return schedule_direction::up;

		//if (&(direction::operator()) == 
		//	&(std::less<int>::operator())) // compare the comparison functions
		//	return schedule_direction::down;
		//else
		//	return schedule_direction::up;

		if (std::is_same<direction, DOWN>::value)
			return schedule_direction::down;
		else
			return schedule_direction::up;

	}

	template<class direction>
	bool elevator_schedule<direction>::has_capacity(int from_floor, int to_floor, int passenger_numbers)
	{
		// check direction first
		if (from_floor == to_floor)
			return false;

		if ((get_direction() == schedule_direction::down) && (from_floor < to_floor))
			return false;

		if ((get_direction() == schedule_direction::up) && (to_floor < from_floor))
			return false;


		int floor_count_expected = std::abs(from_floor - to_floor) + 1;
		int floor_count_actual = 0;

		auto itr = schedule.begin();
		auto back_itr = schedule.rbegin();
		//if ((itr->second->floor > back_itr->second->floor)
		//{
		//	// then down 
		//}
		//schedule.
		while (itr->first != from_floor && itr != schedule.end()) { itr++; }
		for (; itr != schedule.end() && itr->first <= to_floor; itr++)
		{
			if (itr->second->unused_capacity() < passenger_numbers)
				return false;
			floor_count_actual++;
		}
		if ((floor_count_actual - floor_count_expected) != 0)
			return false; // this would happen if called from/to in the wrong direction for this schedule
		else
			return true;
	}

	template<class direction>
	int elevator_schedule<direction>::max_capacity(int from_floor, int to_floor)
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

	//template<class direction>
	//void elevator_schedule<direction>::insert_journey(std::unique_ptr<journey> journey)
	//{
	//	if (!has_capacity(journey->from_floor, journey->to_floor, 1))
	//		throw("no capacity");
	//
	//	return;
	//
	//	//while
	//	//	(
	//	//	(waiting_journey_queue.size() > 0)
	//	//		&& (up_schedule[current_floor]->unused_capacity != 0)
	//	//		)
	//	//{
	//	//	auto passenger = waiting_journey_queue.front()->passenger;
	//	//	int to_floor = waiting_journey_queue.front()->to_floor;
	//
	//	//	up_schedule[current_floor]->pickup_list.push_back(passenger);
	//	//	up_schedule[current_floor]->unused_capacity--;
	//
	//	//	up_schedule[to_floor]->dropoff_list.push_back(std::move(waiting_journey_queue.front())); // schedule this journey
	//
	//	//	waiting_journey_queue.pop();
	//
	//	//	if (current_floor < max_floor)
	//	//	{
	//	//		int next_floor = current_floor + 1;
	//	//		up_schedule[next_floor]->unused_capacity
	//	//			= up_schedule[current_floor]->unused_capacity + up_schedule[next_floor]->dropoff_list.size();
	//	//	}
	//	//}
	//}
}