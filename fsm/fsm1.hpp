#pragma once

#include <iostream>
#include <string>

struct Event {
	int payload;
};

typedef void (*state_f)(const Event &);





class fsm1
{
public:

	fsm1()
	{
		current_state = &fsm1::state_init;
	};

	typedef void(fsm1::* state_f)(const Event& e);
	
	//void (*current_state)(const Event& e);
	//state_f current_state;
	//void (*current_state)(const Event &e);

	state_f current_state;

	void state_init(const Event& e);
	void state_1(const Event& e);
	void state_2(const Event& e);
	void state_3(const Event& e);

	void react(const Event& e);

	void print(std::string state, const Event& e)
	{
		std::cout << "state: " << state << ", event: " << e.payload << std::endl;
	}

};



