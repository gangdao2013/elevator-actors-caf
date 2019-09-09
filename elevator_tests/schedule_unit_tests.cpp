#include "pch.h"
#include "CppUnitTest.h"

#include "elevator/schedule.hpp"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace schedule
{
	TEST_CLASS(schedule_tests)
	{
	public:

		TEST_METHOD(Test_has_capacity)
		{

			schedule::elevator_schedule<UP> schedule;

			bool result = schedule.has_capacity(0, 10, 10);
			Assert::IsTrue(result);
			
		}
	};
}
