#pragma once

#include <string>
//#include "elevator_actor.hpp"

namespace elevator
{


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

		static std::shared_ptr<initialising_state> initalising;
		static std::shared_ptr<disconnected_state> disconnected;
		static std::shared_ptr<idle_state> idle;
		static std::shared_ptr<in_transit_state> in_transit;
		static std::shared_ptr<at_waypoint_state> at_waypoint;
		static std::shared_ptr<quitting_state> quitting;
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
		virtual std::string get_state_name() override { return "at_waypoint"; };
	};

	class quitting_state : public elevator_fsm
	{
		virtual void on_enter(elevator_actor& actor) override;
		virtual std::string get_state_name() override { return "quitting"; };
	};


}
