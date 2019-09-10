## Overview

The application models elevators using CAF actors. It utilises the actor model to achieve concurrent processing, without the hassle of thread management, resource contention, etc.

Broadly:

* controller*.cpp: The code for elevator controllers and their REPL actors.
* elevator*.cpp: The code for elevator actors, their associated finite state machines (FSMs) and their REPL actors.
* passenger*.cpp: The code for passenger actors, their associated finite state machines (FSMs) and their REPL actors.
* dispatcher_actor.cpp: The code for scheduling and dispatching passenger journeys to elevators.

Also, see the code in elevator/schedule.hpp for the data structures and algorithms relating to scheduling.

The scheduling algorithm, broadly, revolves around the concept of up/down schedule classes, containing ordered lists of schedule items for each floor in a building. A schedule item contains information about passenger pickups and drop offs at its floor. The schedule class manages and checks capacity as passenger journey requests come in. If there is sufficient capacity (i.e. the elevator is not overloaded at any stage) then a journey is accepted into the schedule. If there is insufficient capacity then the journey is added to a later departing schedule.

As schedules are finalised, on a timer basis for now, they are dispatched to the first available idle elevator. The dispatch process favours downwards schedules - a future refinement would be to have more sophisticated dispatch based on demand, wait times, time of day, etc.
