#pragma once

#include <string>
//#include "elevator_actor.hpp"

namespace elevator
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
	//                    | /idle       |<---------+       
	//	                  +-------------+          |
	//	                        |                  |
	//       {inst arrives}     |                  | {arrived at destination}
	//                          V                  |
	//                    +-------------+          |
	//                    |             |          |
	//                    | in transit  |----------+
	//                    +-------------+

	class elevator_actor;

	enum class elevator_event_type
	{
		initiate,
		initialised_ok,
		connect,
		connected_ok,
		connection_fail,
		disconnect,
		disconnected,
		//call,
		//elevator_arrived,
		destination_arrived,
		quit
	};

	struct elevator_event
	{
		const elevator_actor& actor;
		elevator_event_type event_type;
	};


	class initialising_state;
	class disconnected_state;
	class connecting_state;
	class idle_state;
	class in_transit_state;
	//class awaiting_instruction_state;
	class quitting_state;

	class elevator_state
	{
	public:
		//virtual ~elevator_state() {};
		virtual void on_enter(elevator_actor& actor) {};
		virtual void on_exit(elevator_actor& actor) {};
		virtual void handle_event(elevator_actor& actor, const elevator_event& event) {};
		virtual std::string get_state_name() { return "elevator_state"; };

		static std::shared_ptr<initialising_state> initalising;
		static std::shared_ptr<disconnected_state> disconnected;
		static std::shared_ptr<connecting_state> connecting;
		static std::shared_ptr<idle_state> idle;
		static std::shared_ptr<in_transit_state> in_transit;
		//static std::shared_ptr<awaiting_instruction_state> awaiting_instruction;
		static std::shared_ptr<quitting_state> quitting;
	};

	class initialising_state : public elevator_state
	{
		virtual void on_enter(elevator_actor& actor) override;
		virtual void handle_event(elevator_actor& actor, const elevator_event& event) override;
		virtual std::string get_state_name() override { return "initialising"; };
	};

	class disconnected_state : public elevator_state
	{
		void on_enter(elevator_actor& actor) override;
		virtual void  handle_event(elevator_actor& actor, const elevator_event& event) override;
		virtual std::string get_state_name() override { return "disconnected"; };
	};

	class connecting_state : public elevator_state
	{
		void on_enter(elevator_actor& actor) override;
		virtual void  handle_event(elevator_actor& actor, const elevator_event& event) override;
		virtual std::string get_state_name() override { return "connecting"; };
	};

	class idle_state : public elevator_state
	{
		void on_enter(elevator_actor& actor) override;
		virtual void handle_event(elevator_actor& actor, const elevator_event& event) override;
		virtual std::string get_state_name() override { return "idle"; };
	};

	class in_transit_state : public elevator_state
	{
		void on_enter(elevator_actor& actor) override;
		virtual void handle_event(elevator_actor& actor, const elevator_event& event) override;
		virtual std::string get_state_name() override { return "in_transit"; };
	};

	//class awaiting_instruction_state : public elevator_state
	//{
	//	void on_enter(elevator_actor& actor) override;
	//	virtual void handle_event(elevator_actor& actor, const elevator_event& event) override;
	//	virtual std::string get_state_name() override { return "awaiting_instruction"; };
	//};

	class quitting_state : public elevator_state
	{
		virtual void on_enter(elevator_actor& actor) override;
		virtual std::string get_state_name() override { return "quitting"; };
	};

}
