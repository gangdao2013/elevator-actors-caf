#include "pch.h"
#include "CppUnitTest.h"

#include "elevator/schedule.hpp"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace schedule
{
	TEST_CLASS(schedule_tests)
	{
	public:

		TEST_METHOD(Test_has_capacity_up)
		{
			// Test the elevator_schedule class capacity methods in various combinations

			// note the use of the parameterised std::string type - we don't have to test with actor objects
			schedule::elevator_schedule<std::string, UP> schedule; 

			bool result = schedule.has_capacity(BOTTOM_FLOOR, TOP_FLOOR, ELEVATOR_CAPACITY_MAX);
			Assert::IsTrue(result);

			result = schedule.has_capacity(BOTTOM_FLOOR+3, TOP_FLOOR-2, ELEVATOR_CAPACITY_MAX);
			Assert::IsTrue(result);

			result = schedule.has_capacity(BOTTOM_FLOOR, TOP_FLOOR, ELEVATOR_CAPACITY_MAX + 3);
			Assert::IsFalse(result);

			result = schedule.has_capacity(BOTTOM_FLOOR+3, TOP_FLOOR-2, ELEVATOR_CAPACITY_MAX + 3);
			Assert::IsFalse(result);

			result = schedule.has_capacity(TOP_FLOOR, BOTTOM_FLOOR, ELEVATOR_CAPACITY_MAX);
			Assert::IsFalse(result);

			result = schedule.has_capacity(TOP_FLOOR, BOTTOM_FLOOR, ELEVATOR_CAPACITY_MAX + 7);
			Assert::IsFalse(result);
			
		}

		TEST_METHOD(Test_has_capacity_down)
		{
			// Test the elevator_schedule class capacity methods in various combinations

			// note the use of the parameterised std::string type - we don't have to test with actor objects
			schedule::elevator_schedule<std::string, DOWN> schedule;

			bool result = schedule.has_capacity(TOP_FLOOR, BOTTOM_FLOOR, ELEVATOR_CAPACITY_MAX);
			Assert::IsTrue(result);

			result = schedule.has_capacity(TOP_FLOOR-3, BOTTOM_FLOOR+2, ELEVATOR_CAPACITY_MAX);
			Assert::IsTrue(result);

			result = schedule.has_capacity(TOP_FLOOR, BOTTOM_FLOOR, ELEVATOR_CAPACITY_MAX + 3);
			Assert::IsFalse(result);

			result = schedule.has_capacity(TOP_FLOOR-3, BOTTOM_FLOOR+2, ELEVATOR_CAPACITY_MAX + 3);
			Assert::IsFalse(result);

			result = schedule.has_capacity(BOTTOM_FLOOR, TOP_FLOOR, ELEVATOR_CAPACITY_MAX);
			Assert::IsFalse(result);

			result = schedule.has_capacity(BOTTOM_FLOOR, TOP_FLOOR, ELEVATOR_CAPACITY_MAX + 7);
			Assert::IsFalse(result);

		}

		TEST_METHOD(Test_max_capacity_down)
		{
			// Test the elevator_schedule class capacity methods in various combinations

			// note the use of the parameterised std::string type - we don't have to test with actor objects
			schedule::elevator_schedule<std::string, DOWN> schedule;

			int capacity = schedule.max_capacity(TOP_FLOOR, BOTTOM_FLOOR);
			Assert::AreEqual(ELEVATOR_CAPACITY_MAX, capacity);

			schedule.insert_journey("Passenger Down", 6, 3);
			capacity = schedule.max_capacity(6, 3);
			Assert::AreEqual(ELEVATOR_CAPACITY_MAX - 1, capacity);

		}

		TEST_METHOD(Test_max_capacity_up)
		{
			// Test the elevator_schedule class capacity methods in various combinations

			// note the use of the parameterised std::string type - we don't have to test with actor objects
			schedule::elevator_schedule<std::string, UP> schedule;

			int capacity = schedule.max_capacity(BOTTOM_FLOOR, TOP_FLOOR);
			Assert::AreEqual(ELEVATOR_CAPACITY_MAX, capacity);

			schedule.insert_journey("Passenger Up", 3, 6);
			capacity = schedule.max_capacity(3, 6);
			Assert::AreEqual(ELEVATOR_CAPACITY_MAX - 1, capacity);

		}
	};
}
