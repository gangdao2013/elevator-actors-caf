#include "pch.h"
#include "CppUnitTest.h"

#include "elevator/schedule.hpp"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace elevator
{
	using DOWN = std::less<int>;
	using UP = std::greater<int>;

	TEST_CLASS(schedule_unit_tests)
	{
	public:
	
		TEST_METHOD(Test_has_capacity)
		{

			schedule::elevator_schedule<DOWN> schedule;

			bool result = schedule.has_capacity(10, 0,3);
			Assert::IsTrue(result);
			
		}
	};
}
