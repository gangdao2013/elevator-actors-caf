#pragma once

#include <memory>
#include <vector>
#include <map>
#include <algorithm>
//#include <math.h>
#include <queue>

#include "elevator/elevator.hpp"
#include "caf/all.hpp"

using namespace elevator;



namespace schedule
{

	//using pickup_list_t<P> = std::vector<P>;
	//using dropoff_list_t<P> = std::vector<P>;

	using UP = std::less<int>;
	using DOWN = std::greater<int>;

	enum class schedule_direction
	{
		up,
		down
	};

	template<class P>
	struct elevator_schedule_item
	{
		int floor{ 0 };
		int onboard_count{ 0 };
		std::vector<P> pickup_list;
		std::vector<P> dropoff_list;
		inline int unused_capacity();
	};

	template<class P>
	struct elevator_waypoint_item
	{
		int floor{ 0 };
		std::vector<P> pickup_list;
		std::vector<P> dropoff_list;
	};

	template<class P>
	inline int elevator_schedule_item<P>::unused_capacity()
	{
		int result = ELEVATOR_CAPACITY_MAX - onboard_count - pickup_list.size() + dropoff_list.size();
		return result;
	}

	template<class P, class direction>
	class elevator_schedule
	{
	public:

		//elevator_schedule();
		elevator_schedule()
		{
			for (int i = 0; i < MAX_FLOORS; i++)
			{
				schedule[i] = std::move(std::make_unique<elevator_schedule_item<P>>());
				schedule[i]->floor = i;
				schedule[i]->onboard_count = 0;
			}
		}
		//~elevator_schedule();

		bool has_capacity(int from_floor, int to_floor, int passenger_numbers);
		int max_capacity(int from_floor, int to_floor);
		schedule_direction get_direction();
		void insert_journey(P passenger, int from_floor, int to_floor);
		std::queue<std::unique_ptr<elevator_waypoint_item<P>>> get_waypoints_queue();

	private:

		std::map<int, std::unique_ptr<elevator_schedule_item<P>>, direction> schedule;

	};

	//template<class P, class direction>
	//elevator_schedule<class P, direction>::~elevator_schedule()
	//{
	//}

	// NB: move semantics:
	template<class P, class direction>
	std::queue<std::unique_ptr<elevator_waypoint_item<P>>> elevator_schedule<P, direction>::get_waypoints_queue()
	{
		std::queue<std::unique_ptr<elevator_waypoint_item<P>>> queue;
		if (schedule.size() > 0)
		{
			for (auto itr = schedule.begin(); itr != schedule.end(); itr++)
			{
				// only interested in floors that have pickups/dropoffs
				if (itr->second->pickup_list.size() > 0
					|| itr->second->dropoff_list.size() > 0)
				{
					auto waypoint = std::make_unique<elevator_waypoint_item<P>>();
					waypoint->floor = itr->second->floor;
					waypoint->pickup_list = std::move(itr->second->pickup_list);
					waypoint->dropoff_list = std::move(itr->second->dropoff_list);
					queue.push(std::move(waypoint));
				}
			}
		}
		return queue;
	}

	template<class P, class direction>
	schedule_direction elevator_schedule<P, direction>::get_direction()
	{
		if (std::is_same<direction, DOWN>::value)
			return schedule_direction::down;
		else
			return schedule_direction::up;
	}

	template<class P, class direction>
	bool elevator_schedule<P, direction>::has_capacity(int from_floor, int to_floor, int passenger_numbers)
	{
		// check direction first
		if (from_floor == to_floor)
			return false;

		if ((get_direction() == schedule_direction::down) && (from_floor < to_floor))
			return false;

		if ((get_direction() == schedule_direction::up) && (to_floor < from_floor))
			return false;

		auto itr = schedule.begin();
		while (itr->first != from_floor && itr != schedule.end()) { itr++; }
		for (; itr != schedule.end(); itr++)
		{
			if (itr->second->unused_capacity() < passenger_numbers)
				return false;
			if (itr->first == to_floor)
				break;
		}
		return true;
	}

	template<class P, class direction>
	int elevator_schedule<P, direction>::max_capacity(int from_floor, int to_floor)
	{
		if (from_floor == to_floor)
			return schedule[from_floor]->unused_capacity();

		if ((get_direction() == schedule_direction::down) && (from_floor < to_floor))
			return 0;

		if ((get_direction() == schedule_direction::up) && (to_floor < from_floor))
			return 0;

		int max_capacity = ELEVATOR_CAPACITY_MAX;

		auto itr = schedule.begin();
		while (itr->first != from_floor && itr != schedule.end()) { itr++; }
		for (; itr != schedule.end(); itr++)
		{
			if (itr->second->unused_capacity() < max_capacity)
				max_capacity = itr->second->unused_capacity();
			if (itr->first == to_floor)
				break;
		}
		return max_capacity;
	}

	template<class P, class direction>
	void elevator_schedule<P, direction>::insert_journey(P passenger, int from_floor, int to_floor)
	{
		if (!has_capacity(from_floor, to_floor, 1))
			throw("no capacity");
	
		schedule[from_floor]->pickup_list.push_back(passenger);
		schedule[to_floor]->dropoff_list.push_back(passenger);

		auto itr = schedule.begin();
		// seek to the next floor after from_floor
		while (itr->first != from_floor && itr != schedule.end()) { itr++; } 
		itr++;
		// update the onboard counts, up to and including to_floor. NB, unused capacity = MAX - onboard - pickups + dropoffs
		for (; itr != schedule.end(); itr++)
		{
			itr->second->onboard_count++;
			if (itr->first == to_floor)
				break;
		}
	}
}