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
	// passenter_actor represents passengers waiting in lobbys or in elevators, moving up and down.
	// They make calls for floors to controller_actors, which in turn delegate to a dispatcher_actor for 
	// scheduling and dispatch.

	// Passengers work hand-in-glove with an embedded finite state machine (FSM) passenger_fsm, to respond
	// appropriately to incoming messages, events, etc., based on the current state.
	// See passenger_fsm.hpp/cpp for more details.

	passenger_actor::passenger_actor(actor_config& cfg, std::string name) :
		event_based_actor(cfg)
		, cfg_{ cfg }
		, name{ name }
	{
		// start off on the right foot...
		transition_to_state(passenger_fsm::initalising);

		set_default_handler([=](scheduled_actor* actor, message_view& view)
			{
				aout(this) << "passenger_actor: unknown message" << std::endl;
				return sec::unexpected_message;
			});

	}

	// override for actor behaviour
	// note that messages received by the actor are mostly delegated to the FSM
	behavior passenger_actor::make_behavior()
	{
		return {
			[=](connect_to_controller_atom, std::string host, uint16_t port)
			{
				aout(this) << "\npassenger: connect_to_controller_atom received, host: " << host << ", port: " << port << endl;
				fsm->handle_connect(*this, host, port);
			},
			[=](register_dispatcher_atom)
			{
				debug_msg("register_dispatcher_atom received");
				dispatcher = std::move(current_sender());
			},
			[=](call_atom, int to_floor)
			{
				aout(this) << "\npassenger: call_atom received, for floor: " << to_floor << endl;
				fsm->handle_call(*this, current_floor, to_floor);
			},
			[=](quit_atom)
			{
				aout(this) << "\npassenger: quit_atom received" << endl;
				fsm->handle_quit(*this);
			},
			[=](embark_atom, int elevator_number)
			{
				aout(this) << "\npassenger: embark_atom received" << endl;
				fsm->handle_elevator_arrived(*this, elevator_number);
			},
			[=](disembark_atom, int floor)
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
			[=](elevator::subscribe_atom, std::string subscriber_key, elevator_observable_event_type event_type)
			{
				add_subscriber(current_sender(), subscriber_key, event_type);
				debug_msg("subscribe_atom received");
			}
		};
	}

	// send debug message to all registered subscribers
	void passenger_actor::debug_msg(std::string msg)
	{
		string subscriber_msg = "[passenger][" + name + "][" + fsm->get_state_name() + "][" + std::to_string(current_floor) + "]: " + msg;
		for (auto kv : debug_message_subscribers)
		{
			auto recipient = actor_cast<actor>(kv.second);
			send(recipient, message_atom::value, subscriber_msg);
		}

	}

	// register debug message subscriber
	void passenger_actor::add_subscriber(strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type)
	{
		// add subscriber to relevant subscriber map
		switch (event_type)
		{
		case elevator_observable_event_type::debug_message:
		{
			// nb: deliberately replace if key is the same, need to destroy any existing ref
			auto existing_ptr = debug_message_subscribers[subscriber_key];
			if (existing_ptr)
			{
				auto handle = actor_cast<actor>(existing_ptr);
				destroy(handle);
			}
			debug_message_subscribers[subscriber_key] = std::move(subscriber);
		}
		break;
		default:
			break;
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

	void passenger_actor::on_connect(const std::string& host, uint16_t port)
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
					if (!controller)
					{
						aout(this) << R"(*** no controller found at ")" << host << R"(":)"
							<< port << endl;
						return;
					}
					if (!ifs.empty())
					{
						aout(this) << R"(*** typed actor found at ")" << host << R"(":)"
							<< port << ", but expected an untyped actor " << endl;
						return;
					}
					aout(this) << "*** successfully connected to controller" << endl;
					controller_host.assign(host);
					controller_port = port;
					this->controller = controller;
					auto controller_hdl = actor_cast<actor>(controller);
					this->monitor(controller_hdl);
					this->send(controller, elevator::register_passenger_atom::value);
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
	}

	void passenger_actor::transition_to_state(std::shared_ptr<passenger_fsm> state)
	{
		if (this->fsm)
			this->fsm->on_exit(*this); // any previous state exit code
		this->fsm = state;
		this->fsm->on_enter(*this); // an next state entry code
	}

	void passenger_actor::on_call(int from_floor, int to_floor)
	{
		// sanity check
		if (from_floor > elevator::TOP_FLOOR
			|| from_floor < elevator::BOTTOM_FLOOR
			|| to_floor > elevator::TOP_FLOOR
			|| to_floor < elevator::BOTTOM_FLOOR
			)
			return;

		if (dispatcher)
		{
			send(dispatcher, elevator::call_atom::value, from_floor, to_floor); 
			// dispatcher will take it from here, scheduling the requested journey
		}
	}

	// we've arrived
	void passenger_actor::on_arrive(int arrived_at_floor)
	{
		if (arrived_at_floor > elevator::TOP_FLOOR || arrived_at_floor < elevator::BOTTOM_FLOOR)
			return;
		current_floor = arrived_at_floor;
	}

	// into the elevator lobby
	void passenger_actor::on_lobby()
	{
		aout(this) << "\npassenger: stepping into lobby" << endl;
	}

	// into the elevator
	void passenger_actor::on_elevator()
	{
		aout(this) << "\npassenger: stepping into elevator" << endl;
	}

}
