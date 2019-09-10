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

/*

Namespace schedule contains the data structures and algorithms for scheduling passenger journeys.
Note that many of the data structures and code are templates - this is to allow flexibility of
testing the code without having to use an actual actor sub-system; e.g. you can just test with template passenger parameters to std::string.

*/

namespace schedule
{

	// These aliases are used to differentiate schedule directions, effected as sorted maps.

	using UP = std::less<int>;
	using DOWN = std::greater<int>;

	enum class schedule_direction
	{
		up,
		down
	};

	// Schedule items appear in schedules - used to track waypoint floors, how many onboard, pickups and dropoffs.
	// There is a schedule item for each floor of the building, for each of the schedules; some might have empty pickups/dropoffs.
	// This is necessary so we can calculate and recalculate schedule capacities as passenger journey calls arrive.

	template<class P>
	struct elevator_schedule_item
	{
		int floor{ 0 };
		int onboard_count{ 0 };
		std::vector<P> pickup_list;
		std::vector<P> dropoff_list;
		inline int unused_capacity();
	};

	// Simplified data structure for holding just waypoints and pickup/dropoff lists, holding only those floors that have pickups/dropoffs.
	// This is what is dispatched to an elevator, once a schedule is finalised.
	template<class P>
	struct elevator_waypoint_item
	{
		int floor{ 0 };
		std::vector<P> pickup_list;
		std::vector<P> dropoff_list;
	};


	// Calculate the unused capacity for a shedule item (floor)
	template<class P>
	inline int elevator_schedule_item<P>::unused_capacity()
	{
		int result = ELEVATOR_CAPACITY_MAX - onboard_count - pickup_list.size() + dropoff_list.size();
		return result;
	}

	// elevator_schedule - used to hold journey & capacity information
	template<class P, class direction>
	class elevator_schedule
	{
	public:

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

		// When a schedule is dispatched, it is moved into an elevator_waypoint_item queue, passed on to 
		// the elevator being dispatched to.
		std::queue<std::unique_ptr<elevator_waypoint_item<P>>> get_waypoints_queue();

	private:

		// Schedule - dynamically updated up until it is dispatched.
		std::map<int, std::unique_ptr<elevator_schedule_item<P>>, direction> schedule;

	};

	//template<class P, class direction>
	//elevator_schedule<class P, direction>::~elevator_schedule()
	//{
	//}


	// Finalise and extract waypoint items from the schedule. 
	// NB: move semantics: the schedule will be empty after this - should be disposed.
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


	// Get the direction of the schedule
	template<class P, class direction>
	schedule_direction elevator_schedule<P, direction>::get_direction()
	{
		if (std::is_same<direction, DOWN>::value) // compare comparators
			return schedule_direction::down;
		else
			return schedule_direction::up;
	}

	// Does this schedule have capacity for a trip from_floor to to_floor for supplied passenger_numbers. True: yes, False: no.
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

		// Capacity availability is determined just by iterating through the relevant floors and checking 
		// all the schedule_items have spare capacity.
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

	// Return the maximum capacity over a route.
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

	// Insert this passenger journey into the schedule. Calling code should first check if there is capacity (has_capacity).
	// Will throw an exception if not.
	template<class P, class direction>
	void elevator_schedule<P, direction>::insert_journey(P passenger, int from_floor, int to_floor)
	{
		if (!has_capacity(from_floor, to_floor, 1))
			throw("no capacity");
	
		// set pickup/dropoff lists
		schedule[from_floor]->pickup_list.push_back(passenger);
		schedule[to_floor]->dropoff_list.push_back(passenger);

		// adjust the capacity of the schedule between from/to floors:

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