#pragma once

#include <string>
#include "elevator/elevator_actor.hpp"

namespace elevator
{

	/*
	
	Elevator finite state machine; broadly based on GOF State pattern, with a major difference being
	that events are reified as calls to the various handle_* functions. States & substates then override these
	handler functions as appropriate.

	Generally, the handler functions call back into the elevator actor to have it respond - in this sense the FSM is 
	a coordinator of elevator actor actions.

	Note: many of the CAF examples achieve FSM-like behaviour by switching around message_handlers; I prefer this approach as 
	it is more testable, and involves less code duplication.
	
	*/


	// State transition for the elevator for connecting to the elevator controller, making calls, in transit, etc:
	//
	//                    +-------------+
	//                    |    init     |
	//                    +-------------+
	//                           |
	//                           V
	//                    +-------------+
	//                    | unconnected |<------------------+
	//                    +-------------+                   |
	//                           |                          |
	//                           | {connect Host Port}      |
	//                           |                          |
	//                           V                          |
	//                    +-------------+  {error}          |
	//    +-------------->| connecting  |-------------------+
	//    |               +-------------+                   |
	//    |                      |                          |
	//    |                      | {ok, connected}          |
	//    |{connect Host Port}   |                          |
	//    |                      V                          |
	//    |               +-------------+ {DOWN controller} |
	//    +---------------|   idle      |-------------------+
	//                    |             |<---------+       
	//	                  +-------------+          |
	//	                        |                  |
	//            {waypoint}    |                  | {no waypoints}
	//                          V                  |
	//                    +-------------+       +--------------+
	//                    |             |------>|              |
	//    {waypoint}----->| in transit  |<------| at waypoint  |<---- {waypoint}
	//                    +-------------+       +--------------+

	
	class elevator_actor;

	
	class initialising_state;
	class disconnected_state;
	
	class waypoint_accepting_state;
	
	class idle_state;
	class in_transit_state;
	class at_waypoint_state;
	
	class quitting_state;

	class elevator_fsm
	{
	public:
		//virtual ~elevator_state() {};
		virtual void on_enter(elevator_actor& actor) {};
		virtual void on_exit(elevator_actor& actor) {};

		// FSM event handlers

		virtual void handle_initialise(elevator_actor& actor) {};
		virtual void handle_connect(elevator_actor& actor, std::string host, uint16_t port);
		virtual void handle_start(elevator_actor& actor) {};
		virtual void handle_waypoint_received(elevator_actor& actor, int waypoint_floor) {};
		virtual void handle_timer(elevator_actor& actor) {};
		virtual void handle_quit(elevator_actor& actor);

		virtual std::string get_state_name() { return "elevator_state"; };

		static const std::shared_ptr<initialising_state> initalising;
		static const std::shared_ptr<disconnected_state> disconnected;
		static const std::shared_ptr<idle_state> idle;
		static const std::shared_ptr<in_transit_state> in_transit;
		static const std::shared_ptr<at_waypoint_state> at_waypoint;
		static const std::shared_ptr<quitting_state> quitting;
	};

	// FSM States

	class initialising_state : public elevator_fsm
	{
		virtual void on_enter(elevator_actor& actor) override;
		virtual std::string get_state_name() override { return "initialising"; };
	};

	class disconnected_state : public elevator_fsm
	{
		virtual std::string get_state_name() override { return "disconnected"; };
	};

	class waypoint_accepting_state : public elevator_fsm {
		virtual void handle_waypoint_received(elevator_actor& actor, int waypoint_floor) override;
	};

	class idle_state : public waypoint_accepting_state
	{
		virtual void on_enter(elevator_actor& actor) override;
		virtual void handle_start(elevator_actor& actor) override;
		virtual std::string get_state_name() override { return "idle"; };
	};

	class in_transit_state : public waypoint_accepting_state
	{
		virtual void on_enter(elevator_actor& actor) override;
		virtual void handle_timer(elevator_actor& actor) override;
		virtual std::string get_state_name() override { return "in_transit"; };
	};

	class at_waypoint_state : public waypoint_accepting_state
	{
		virtual void on_enter(elevator_actor& actor) override;
		virtual void handle_timer(elevator_actor& actor) override;
		virtual std::string get_state_name() override { return "at_waypoint"; };
	};

	class quitting_state : public elevator_fsm
	{
		virtual void on_enter(elevator_actor& actor) override;
		virtual std::string get_state_name() override { return "quitting"; };
	};


}
