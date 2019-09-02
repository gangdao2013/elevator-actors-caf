#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "string_util.hpp"

#include "elevator.hpp"

#include "elevator_actor.hpp"
#include "elevator_fsm.hpp"

#include <cassert>

using namespace caf;
using namespace std;
using namespace elevator;


namespace elevator
{

	behavior elevator_actor::make_behavior()
	{
		return {
			[=](elevator::quit_atom)
			{
				aout(this) << "\nelevator: quit_atom received" << endl;
				fsm_->handle_quit(*this);
			},
			[=](connect_to_controller_atom, const std::string& host, uint16_t port)
			{
				aout(this) << "\nelevator: connect_to_controller_atom received, host: " << host << ", port: " << port << endl;
				this->controller_host = host;
				this->controller_port = port;
				fsm_->handle_connect(*this, host, port);
			},
			[=](elevator::waypoint_atom, int waypoint_floor)
			{
				aout(this) << "\nelevator: waypoint_atom received, for floor: " << waypoint_floor << endl;
				fsm_->handle_waypoint_received(*this, waypoint_floor);
			},
			[=](get_current_floor_atom)
			{
				//aout(this) << "\nelevator: get_current_floor_atom received" << endl;
				return current_floor;
			},
			[=](get_current_state_name_atom)
			{
				//aout(this) << "\nelevator: get_current_state_name_atom received" << endl;
				return fsm_->get_state_name();
			}
		};
	}


	void elevator_actor::transition_to_state(std::shared_ptr<elevator_fsm> state)
	{
		//assert(this->state_ != nullptr);
		if (this->fsm_)
			this->fsm_->on_exit(*this);
		this->fsm_ = state;
		this->fsm_->on_enter(*this);
	}

	void elevator_actor::on_quit()
	{

		anon_send_exit(this, exit_reason::user_shutdown);
	}

	bool elevator_actor::on_initialise()
	{
		// transition to `unconnected` on elevator controller failure/shutdown
		// set the handler if we lose connection to elevator controller 
		set_down_handler([=](const down_msg& dm)
			{
				if (dm.source == controller)
				{
					aout(this) << "\nelevator: lost connection to elevator controller, please reconnect or quit" << endl;
					this->controller = nullptr;
				}
			});

		return true;
	}

	void elevator_actor::on_connect(const std::string& host, uint16_t port)
	{
		// make sure we are not pointing to an old controller
		controller = nullptr;
		controller_host.assign("");
		controller_port = 0;

		// use request().await() to suspend regular behavior until MM responded
		auto mm = system().middleman().actor_handle();

		request(mm, infinite, connect_atom::value, controller_host, controller_port)
			.await
			(
				[host, port, this](const node_id&, strong_actor_ptr controller, const std::set<std::string>& ifs)
				{
					if (!controller)
					{
						aout(this) << R"(*** no controller found at ")" << controller_host << R"(":)"
							<< controller_port << endl;
						return;
					}
					if (!ifs.empty())
					{
						aout(this) << R"(*** typed actor found at ")" << controller_host << R"(":)"
							<< controller_port << ", but expected an untyped actor " << endl;
						return;
					}
					aout(this) << "*** successfully connected to controller" << endl;
					controller_host.assign(host);
					controller_port = port;
					this->controller = controller;
					auto controller_hdl = actor_cast<actor>(controller);
					this->monitor(controller_hdl);
					//this->send(controller_hdl, elevator::register_elevator_atom::value, this);
					transition_to_state(elevator_fsm::idle);
				},
				[host, port, this](const error& err)
				{
					aout(this) << R"(*** cannot connect to ")" << controller_host << R"(":)"
						<< controller_port << " => " << this->system().render(err) << endl;
					transition_to_state(elevator_fsm::disconnected);
				}
				);
	}

	void elevator_actor::on_waypoint_received(int waypoint_floor)
	{
		if (waypoint_floor > elevator::FLOOR_MAX || waypoint_floor < elevator::FLOOR_MIN)
			return;
	}

	void elevator_actor::on_idle()
	{
	}

	// start the lift if there are any waypoints
	// return true: has waypoints and ready to go, false: no waypoints
	// FSM only has idle_state calling this
	bool elevator_actor::on_start()
	{
	}

	void elevator_actor::on_in_transit()
	{
	}

	void elevator_actor::on_waypoint_arrive(int waypoint_floor)
	{
	}




}
