#include "fsm1.hpp"
#include <string>

void fsm1::state_init(const Event& e)
{
	fsm1::print("state_init", e);
	current_state = &fsm1::state_1;
}

void fsm1::state_1(const Event& e)
{
	fsm1::print("state_1", e);
	current_state = &fsm1::state_2;
}

void fsm1::state_2(const Event& e)
{
	fsm1::print("state_2", e);
	current_state = &fsm1::state_3;
}

void fsm1::state_3(const Event& e)
{
	fsm1::print("state_3", e);
	current_state = &fsm1::state_1;
}

void fsm1::react(const Event& e)
{
	if (this->current_state)
		(this->*current_state)(e);
}
