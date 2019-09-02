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
			[=](connect_to_controller_atom, std::string host, uint16_t port)
			{
				aout(this) << "\nelevator: connect_to_controller_atom received, host: " << host << ", port: " << port << endl;
				this->controller_host = host;
				this->controller_port = port;
				raise_event(elevator_event{ *this, elevator_event_type::connect });
			},
			[=](elevator::connect_to_controller_atom)
			{
				aout(this) << "\nelevator: connect_to_controller_atom received" << endl;
				raise_event(elevator_event{ *this, elevator_event_type::connect });
			},
			//[=](elevator::call_atom, int to_floor)
			//{
			//	aout(this) << "\nelevator: call_atom received, for floor: " << to_floor << endl;
			//	called_floor = to_floor;
			//	// for now just to test against
			//	// TODO: remove this line
			//	current_floor = to_floor;
			//	raise_event(elevator_event{ *this, elevator_event_type::call });
			//},
			[=](elevator::quit_atom)
			{
				aout(this) << "\nelevator: quit_atom received" << endl;
				raise_event(elevator_event{ *this, elevator_event_type::quit });
			},
			[=](destination_arrived_atom, int floor)
			{
				current_floor = floor;
				aout(this) << "\nelevator: destination_arrived_atom received" << endl;
				raise_event(elevator_event{ *this, elevator_event_type::destination_arrived });
			},
			[=](get_current_floor_atom)
			{
				//aout(this) << "\nelevator: get_current_floor_atom received" << endl;
				return current_floor;
			},
			[=](get_current_state_name_atom)
			{
				//aout(this) << "\nelevator: get_current_state_name_atom received" << endl;
				return state_->get_state_name();
			}
		};
	}

	void elevator_actor::raise_event(const elevator_event& event)
	{
		assert(this->state_ != nullptr);

		state_->handle_event(*this, event);

	}

	void elevator_actor::quit()
	{

		anon_send_exit(this, exit_reason::user_shutdown);
	}

	void elevator_actor::initialise()
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
		raise_event(elevator_event{ *this, elevator_event_type::initialised_ok });
		return;
	}

	void elevator_actor::connect()
	{

		//stateful_actor<state>* self, const std::string& host, uint16_t port
		// make sure we are not pointing to an old controller
		controller = nullptr;

		// use request().await() to suspend regular behavior until MM responded
		auto mm = system().middleman().actor_handle();

		request(mm, infinite, connect_atom::value, controller_host, controller_port)
			.await
			(
				[&](const node_id&, strong_actor_ptr controller, const std::set<std::string>& ifs)
				{
					if (!controller) {
						aout(this) << R"(*** no controller found at ")" << controller_host << R"(":)"
							<< controller_port << endl;
						return;
					}
					if (!ifs.empty()) {
						aout(this) << R"(*** typed actor found at ")" << controller_host << R"(":)"
							<< controller_port << ", but expected an untyped actor " << endl;
						return;
					}
					aout(this) << "*** successfully connected to controller" << endl;
					this->controller = controller;
					//auto controller_hdl = actor_cast<actor>(controller);
					//this->monitor(controller_hdl);
					//this->send(controller_hdl, elevator::register_elevator_atom::value, this);
					raise_event(elevator_event{ *this, elevator_event_type::connected_ok });
				},
				[&](const error& err)
				{
					aout(this) << R"(*** cannot connect to ")" << controller_host << R"(":)"
						<< controller_port << " => " << this->system().render(err) << endl;
					raise_event(elevator_event{ *this, elevator_event_type::connection_fail });
				}
				);
	}

	void elevator_actor::set_state(std::shared_ptr<elevator_state> state)
	{
		//assert(this->state_ != nullptr);
		if (this->state_)
			this->state_->on_exit(*this);
		this->state_ = state;
		this->state_->on_enter(*this);
	}

}
