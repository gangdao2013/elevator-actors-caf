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

			schedule::elevator_schedule<UP> schedule;

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

			schedule::elevator_schedule<DOWN> schedule;

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
	};
}
