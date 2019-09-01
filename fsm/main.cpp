// fsm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "fsm1.hpp"

int main()
{

	fsm1 m1;

	fsm1 m2;


	m1.react(Event{ 10 });
	m2.react(Event{ 20 });

	m1.react(Event{ 11 });
	m2.react(Event{ 21 });

	m1.react(Event{ 12 });
	m2.react(Event{ 22 });

	m1.react(Event{ 13 });
	m2.react(Event{ 23 });

	m1.react(Event{ 14 });
	m2.react(Event{ 24 });

	m1.react(Event{ 15 });
	m2.react(Event{ 25 });

}
