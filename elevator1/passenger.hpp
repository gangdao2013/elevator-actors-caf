#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator.hpp"

namespace passenger
{
	enum class passenger_event_type
	{
		initiate,
		connect,
		connected_ok,
		disconnect,
		disconnected,
		//lift_
		quit
	};


	struct passenger_event
	{
		const passenger_actor& actor;
		passenger_event_type event_type;
	};
	
	void start_passenger(actor_system& system, const elevator::config& cfg);

	class passenger_actor : public event_based_actor
	{

	public:
		passenger_actor(const elevator::config& cfg) : 
			event_based_actor((actor_config&)cfg)
			, cfg_{cfg}
			, current_state { &passenger_actor::initialising }
		{ 
			//cfg_ = cfg ;
			//current_state = &passenger_actor::initialising;
		}

		behavior make_behavior() override;

		void handle_event(const passenger_event& event);

	private:

		const elevator::config& cfg_;
		strong_actor_ptr controller;

		typedef void(passenger_actor::* state_f)(const passenger_event& e);
		state_f current_state = nullptr;

		// states
		void initialising(const passenger_event& e);
		void disconnected(const passenger_event& e);
		//void connecting(const passenger_event& e);
		void connected(const passenger_event& e);
		void in_lobby(const passenger_event& e);
		void in_elevator(const passenger_event& e);

		//

		bool connect();
	};


}
