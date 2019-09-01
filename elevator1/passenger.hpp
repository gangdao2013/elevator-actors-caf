#pragma once

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator.hpp"
#include "tinyfsm.hpp"

namespace passenger
{
	

	void start_passenger(actor_system& system, const elevator::config& cfg);


	class passenger_actor : public event_based_actor
	{
	public:
		passenger_actor(actor_config& cfg) : event_based_actor(cfg), _cfg{ cfg }
		{ 
			initialise.actor = this; 
			initialisedOk.actor = this;
			connect.actor = this;
			connectedOk.actor = this;
			connectFailed.actor = this;
			elevatorAtStartFloor.actor = this;
			elevatorAtDestinationFloor.actor = this;
		}

		behavior make_behavior() override;
	private:
		actor_config& _cfg;
		strong_actor_ptr controller;

		// events
		Initialise initialise;
		InitialisedOk initialisedOk;
		Connect connect;
		ConnectedOk connectedOk;
		ConnectFailed connectFailed;
		ElevatorAtStartFloor elevatorAtStartFloor;
		ElevatorAtDestinationFloor elevatorAtDestinationFloor;

		// states
		void initialising();
	};


	/////////
	// FSM

	// events

	struct PassengerEvent : tinyfsm::Event 
	{
		const passenger_actor * actor;
	};

	struct Initialise : PassengerEvent {};
	struct InitialisedOk : PassengerEvent {};
	struct Connect : PassengerEvent {};
	struct ConnectedOk : PassengerEvent {};
	struct ConnectFailed : PassengerEvent {};
	struct ElevatorAtStartFloor : PassengerEvent {};
	struct ElevatorAtDestinationFloor : PassengerEvent {};

	// states
	class PassengerFsm : public tinyfsm::Fsm<PassengerFsm>
	{
	public:
		// default reaction for unhandled events
		void react(tinyfsm::Event const&) {}; // TODO: diagnostic code here

		virtual void react(Initialise const&);
		virtual void react(InitialisedOk const&);
		virtual void react(Connect const&);
		virtual void react(ConnectedOk const&);
		virtual void react(ConnectFailed const&);


		virtual void entry(void) {};
		virtual void exit(void) {};

	};

	class Initialising : public PassengerFsm 
	{
		void entry() override;
		void react(InitialisedOk const &) override;
	};

	class Disconnected : public PassengerFsm
	{
		void entry() override;
		void react(Connect const&);
	};

	class Connecting : public PassengerFsm
	{
		void entry() override;
		void react(ConnectedOk const&) override;
		void react(ConnectFailed const&) override;
	};

	class InLobby : public PassengerFsm
	{
		void entry() override;
		void react(ElevatorAtStartFloor const&);
	};

	class InElevator : public PassengerFsm
	{
		void entry() override;
		void react(ElevatorAtDestinationFloor);
	};

	//template<typename E>
	//void send_event(E const& event) {
	//	tinyfsm::FsmList<PassengerFsm>::dispatch<E>(event);
	//}

}

FSM_INITIAL_STATE(passenger::PassengerFsm, passenger::Initialising)