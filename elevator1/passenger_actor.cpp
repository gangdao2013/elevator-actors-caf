#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "string_util.hpp"

#include "elevator.hpp"

#include "passenger_actor.hpp"
#include "passenger_fsm.hpp"

#include <cassert>

using namespace caf;
using namespace std;
using namespace elevator;


namespace passenger
{

	behavior passenger_actor::make_behavior()
	{
		return {
			[=](connect_to_controller_atom, std::string host, uint16_t port)
			{
				aout(this) << "\npassenger: connect_to_controller_atom received, host: " << host << ", port: " << port << endl;
				fsm_->handle_connect(*this, host, port);
			},
			//[=](elevator::connect_to_controller_atom)
			//{
			//	aout(this) << "\npassenger: connect_to_controller_atom received" << endl;
			//	raise_event(passenger_event{ *this, passenger_event_type::connect });
			//},
			[=](elevator::call_atom, int to_floor)
			{
				aout(this) << "\npassenger: call_atom received, for floor: " << to_floor << endl;
				fsm_->handle_call(*this, current_floor, to_floor);
			},
			[=](elevator::quit_atom)
			{
				aout(this) << "\npassenger: quit_atom received" << endl;
				fsm_->handle_quit(*this);
			},
			[=](elevator_arrived_atom) {
				aout(this) << "\npassenger: elevator_arrived_atom received" << endl;
				fsm_->handle_elevator_arrived(*this);
			},
			[=](destination_arrived_atom, int floor)
			{
				aout(this) << "\npassenger: destination_arrived_atom received" << endl;
				fsm_->handle_destination_arrived(*this, floor);
			},
			[=](get_current_floor_atom)
			{
				//aout(this) << "\npassenger: get_current_passenger_floor_atom received" << endl;
				return current_floor;
			},
			[=](get_current_state_name_atom)
			{
				//aout(this) << "\npassenger: get_state_name_atom received" << endl;
				return fsm_->get_state_name();
			}
		};
	}

	void passenger_actor::quit()
	{
		anon_send_exit(this, exit_reason::user_shutdown);
	}

	bool passenger_actor::initialise()
	{
		// transition to `unconnected` on elevator controller failure/shutdown
		// set the handler if we lose connection to elevator controller 
		set_down_handler([=](const down_msg& dm)
			{
				if (dm.source == controller)
				{
					aout(this) << "\npassenger: lost connection to elevator controller, please reconnect or quit" << endl;
					this->controller = nullptr;
					transition_to_state(passenger_fsm::disconnected);
				}
			});
		return true;
	}

	void passenger_actor::connect(const std::string &host, uint16_t port)
	{
		//bool result = false;

		// make sure we are not pointing to an old controller
		controller = nullptr;
		controller_host.assign("");
		controller_port = 0;

		// use request().await() to suspend regular behavior until MM responded
		auto mm = system().middleman().actor_handle();

		request(mm, infinite, connect_atom::value, host, port)
			.await
			(
				[host, port, this](const node_id&, strong_actor_ptr controller, const std::set<std::string>& ifs) 
				{
					if (!controller) {
						aout(this) << R"(*** no controller found at ")" << host << R"(":)"
							<< controller_port << endl;
						return;
					}
					if (!ifs.empty()) {
						aout(this) << R"(*** typed actor found at ")" << host << R"(":)"
							<< controller_port << ", but expected an untyped actor " << endl;
						return;
					}
					aout(this) << "*** successfully connected to controller" << endl;
					controller_host.assign(host);
					controller_port = port;
					this->controller = controller;
					auto controller_hdl = actor_cast<actor>(controller);
					this->monitor(controller_hdl);
					//this->send(controller_hdl, elevator::register_passenger_atom::value, this);
					//result = true;
					transition_to_state(passenger_fsm::in_lobby);
				},
				[host, port, this](const error& err) 
				{
					aout(this) << R"(*** cannot connect to ")" << host << R"(":)"
						<< port << " => " << this->system().render(err) << endl;
					transition_to_state(passenger_fsm::disconnected);
				}
			);
		//return result;
	}

	void passenger_actor::transition_to_state(std::shared_ptr<passenger_fsm> state)
	{
		//assert(this->state_ != nullptr);
		if (this->fsm_)
			this->fsm_->on_exit(*this);
		this->fsm_ = state;
		this->fsm_->on_enter(*this);
	}

	bool passenger_actor::call(int from_floor, int to_floor)
	{
		if (controller)
		{
			send(controller, elevator::call_atom::value, from_floor, to_floor);
			return true;
		}
		return false;
	}

	bool passenger_actor::arrive(int arrived_at_floor)
	{
		return true;
	}

	void passenger_actor::into_lobby()
	{
		aout(this) << "\npassenger: stepping into lobby" << endl;
	}

	void passenger_actor::into_elevator()
	{
		aout(this) << "\npassenger: stepping into elevator" << endl;
	}

}
