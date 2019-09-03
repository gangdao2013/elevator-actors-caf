#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "elevator/string_util.hpp"

#include "elevator/elevator.hpp"

#include "elevator/passenger_actor.hpp"
#include "elevator/passenger_fsm.hpp"

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
				fsm->handle_connect(*this, host, port);
			},
			[=](elevator::call_atom, int to_floor)
			{
				aout(this) << "\npassenger: call_atom received, for floor: " << to_floor << endl;
				fsm->handle_call(*this, current_floor, to_floor);
			},
			[=](elevator::quit_atom)
			{
				aout(this) << "\npassenger: quit_atom received" << endl;
				fsm->handle_quit(*this);
			},
			[=](elevator_arrived_atom) {
				aout(this) << "\npassenger: elevator_arrived_atom received" << endl;
				fsm->handle_elevator_arrived(*this);
			},
			[=](destination_arrived_atom, int floor)
			{
				aout(this) << "\npassenger: destination_arrived_atom received" << endl;
				fsm->handle_destination_arrived(*this, floor);
			},
			[=](get_current_floor_atom)
			{
				return current_floor;
			},
			[=](get_current_state_name_atom)
			{
				return fsm->get_state_name();
			},
			[=](get_name_atom)
			{
				return name;
			},
		};
	}

	void passenger_actor::on_quit()
	{
		anon_send_exit(this, exit_reason::user_shutdown);
	}

	bool passenger_actor::on_initialise()
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

	void passenger_actor::on_connect(const std::string &host, uint16_t port)
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
		if (this->fsm)
			this->fsm->on_exit(*this);
		this->fsm = state;
		this->fsm->on_enter(*this);
	}

	void passenger_actor::on_call(int from_floor, int to_floor)
	{
		if (from_floor > elevator::FLOOR_MAX 
			|| from_floor < elevator::FLOOR_MIN
			|| to_floor > elevator::FLOOR_MAX
			|| to_floor < elevator::FLOOR_MIN
			)
			return;

		if (controller)
		{
			send(controller, elevator::call_atom::value, from_floor, to_floor);
		}
	}

	void passenger_actor::on_arrive(int arrived_at_floor)
	{
		if (arrived_at_floor > elevator::FLOOR_MAX || arrived_at_floor < elevator::FLOOR_MIN)
			return;
		current_floor = arrived_at_floor;
	}

	void passenger_actor::on_lobby()
	{
		aout(this) << "\npassenger: stepping into lobby" << endl;
	}

	void passenger_actor::on_elevator()
	{
		aout(this) << "\npassenger: stepping into elevator" << endl;
	}

}
