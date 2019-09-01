#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "string_util.hpp"

#include "elevator.hpp"
#include "controller_actor.hpp"
#include "passenger_actor.hpp"
#include "passenger_fsm.hpp"
#include "passenger_repl.hpp"

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
				this->controller_host = host;
				this->controller_port = port;
				raise_event(passenger_event{ *this, passenger_event_type::connect });
			},
			[=](elevator::connect_to_controller_atom) 
			{ 
				aout(this) << "\npassenger: connect_to_controller_atom received" << endl; 
				raise_event(passenger_event{ *this, passenger_event_type::connect });
			},
			[=](elevator::call_atom, int to_floor) 
			{ 
				aout(this) << "\npassenger: call_atom received, for floor: " << to_floor << endl;
				called_floor = to_floor;
				raise_event(passenger_event{ *this, passenger_event_type::call });
			},
			[=](elevator::quit_atom) 
			{
				aout(this) << "\npassenger: quit_atom received" << endl;
				raise_event(passenger_event{ *this, passenger_event_type::quit });
			},
			[=](elevator_arrived_atom) {
				aout(this) << "\npassenger: elevator_arrived_atom received" << endl;
				raise_event(passenger_event{ *this, passenger_event_type::elevator_arrived });
			},
			[=](destination_arrived_atom, int floor)
			{
				current_floor = floor;
				aout(this) << "\npassenger: destination_arrived_atom received" << endl;
				raise_event(passenger_event{ *this, passenger_event_type::destination_arrived });
			}
		};
	}

	void passenger_actor::raise_event(const passenger_event& event)
	{
		assert(this->state_ != nullptr);

		state_->handle_event(*this, event);

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
			}
		});
		raise_event(passenger_event{ *this, passenger_event_type::initialised_ok });
		return true;
	}

	bool passenger_actor::connect()
	{

		//stateful_actor<state>* self, const std::string& host, uint16_t port
		// make sure we are not pointing to an old controller
		controller = nullptr;

		// use request().await() to suspend regular behavior until MM responded
		auto mm = system().middleman().actor_handle();

		bool connected = false;

		request(mm, infinite, connect_atom::value, controller_host, controller_port)
			.await
			(
				[&](const node_id&, strong_actor_ptr controller,
					const std::set<std::string>& ifs) {
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
						auto controller_hdl = actor_cast<actor>(controller);
						this->monitor(controller_hdl);
						this->send(controller_hdl, elevator::register_passenger_atom::value, this);
						connected = true;
				},
				[&](const error& err) {
					aout(this) << R"(*** cannot connect to ")" << controller_host << R"(":)"
						<< controller_port << " => " << this->system().render(err) << endl;

				}
			);
	
		if(connected)
			raise_event(passenger_event{ *this, passenger_event_type::connected_ok });
		else
			raise_event(passenger_event{ *this, passenger_event_type::connection_fail });

		return connected;
	}

	//auto repl = self->spawn(passenger_repl);
	//self->send(repl, elevator::get_instructions_atom::value, self->state.current_floor);


	bool passenger_actor::get_instruction()
	{
		auto repl = this->spawn<passenger_repl>();
		this->send(repl, infinite, elevator::get_instructions_atom::value, current_floor);
		return true;
	}

	void passenger_actor::set_state(std::shared_ptr<passenger_state> state)
	{
		//assert(this->state_ != nullptr);
		if(this->state_)
			this->state_->on_exit(*this);
		this->state_ = state;
		this->state_->on_enter(*this);
	}

}
