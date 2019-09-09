#pragma once

#include <string>
#include "elevator/passenger_actor.hpp"

namespace passenger
{


	// State transition for the passenger for connecting to the elevator controller, making calls, in transit, etc:
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
	//    +---------------|   waiting   |-------------------+
	//                    | in lobby    |<---------+       
	//	                  +-------------+          |
	//	                        |                  |
	//       {lift arrives}     |                  | {arrived at destination/disembark}
	//                          V                  |
	//                    +-------------+          |
	//                    | in lift/    |          |
	//                    | in transit  |----------+
	//                    +-------------+

	class passenger_actor;

	class initialising_state;
	class disconnected_state;
	class in_lobby_state;
	class in_elevator_state;
	class quitting_state;

	class passenger_fsm
	{
	public:
		//virtual ~passenger_state() {};

		// FSM Behaviours

		virtual void on_enter(passenger_actor& actor) {};		
		virtual void on_exit(passenger_actor& actor) {};

		// FSM event handlers

		virtual void handle_initialise(passenger_actor& actor) {};
		virtual void handle_connect(passenger_actor& actor, std::string host, uint16_t port);
		virtual void handle_call(passenger_actor& actor, int from_floor, int to_floor) {};
		virtual void handle_elevator_arrived(passenger_actor& actor, int elevator_number) {};
		virtual void handle_destination_arrived(passenger_actor& actor, int arrived_at_floor) {};
		virtual void handle_quit(passenger_actor& actor);
		
		virtual std::string get_state_name() { return "passenger_state"; };

		// FSM static state object pointers

		static std::shared_ptr<initialising_state> initalising;
		static std::shared_ptr<disconnected_state> disconnected;
		static std::shared_ptr<in_lobby_state> in_lobby;
		static std::shared_ptr<in_elevator_state> in_elevator;
		static std::shared_ptr<quitting_state> quitting;
	};

	// FSM States

	class initialising_state : public passenger_fsm
	{
		virtual void on_enter(passenger_actor& actor) override;
		virtual std::string get_state_name() override { return "initialising"; };
	};

	class disconnected_state : public passenger_fsm
	{
		virtual std::string get_state_name() override { return "disconnected"; };
	};

	class in_lobby_state : public passenger_fsm
	{
		void on_enter(passenger_actor& actor) override;

		virtual void handle_call(passenger_actor& actor, int from_floor, int to_floor) override;
		virtual void handle_elevator_arrived(passenger_actor& actor, int elevator_number) override;

		virtual std::string get_state_name() override { return "in_lobby"; };
	};

	class in_elevator_state : public passenger_fsm
	{
		void on_enter(passenger_actor& actor) override;
		virtual void handle_destination_arrived(passenger_actor& actor, int arrived_at_floor) override;
		virtual std::string get_state_name() override { return "in_elevator"; };
	};

	class quitting_state : public passenger_fsm
	{
		virtual void on_enter(passenger_actor& actor) override;
		virtual std::string get_state_name() override { return "quitting"; };
	};

}
