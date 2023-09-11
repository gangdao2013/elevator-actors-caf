#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

#include "elevator/string_util.hpp"
#include "elevator/elevator.hpp"

#include "elevator/elevator_actor.hpp"
#include "elevator/elevator_fsm.hpp"

#include <cassert>

using namespace caf;
using namespace std;
using namespace elevator;


namespace elevator
{

	// elevator_actor represents elevators moving up and down.
	// They are scheduled by dispatcher_actor and class scheduler.
	// Elevators are 'tasked' with schedules of floor waypoints; the dispatcher maintains lists of pickups and dropoffs
	// for each floor in any schedule.

	// Elevators work hand-in-glove with an embedded finite state machine (FSM) elevator_fsm, to respond
	// appropriately to incoming messages, events, etc., based on the current state.
	// See elevator_fsm.cpp for more details.


	elevator_actor::elevator_actor(actor_config& cfg, int elevator_number) :
		event_based_actor(cfg)
		, cfg_{ cfg }
		, elevator_number{ elevator_number }
	{
		transition_to_state(elevator_fsm::initalising);
		set_error_handler([=](scheduled_actor* actor, error& err) -> void
			{
				aout(this) << "elevator_actor: error: " << err << std::endl;
			});
	}

	// override for actor behaviour
	// note that messages received by the actor are mostly delegated to the FSM
	behavior elevator_actor::make_behavior()
	{
		return {
			[=](quit_atom)
			{
				debug_msg("quit_atom received");
				fsm->handle_quit(*this);
			},
			[=](connect_to_controller_atom, const std::string& host, uint16_t port)
			{
				debug_msg("connect_to_controller_atom received, host: " + host + ", port: " + std::to_string(port));
				this->controller_host = std::move(host);
				this->controller_port = port;
				fsm->handle_connect(*this, host, port);
			},
			[=](register_dispatcher_atom)
			{
				debug_msg("register_dispatcher_atom received");
				dispatcher = std::move(current_sender());
			},
			[=](waypoint_atom, int waypoint_floor)
			{
				debug_msg("waypoint_atom received, for floor: " + std::to_string(waypoint_floor));
				fsm->handle_waypoint_received(*this, waypoint_floor);
			},
			[=](get_current_floor_atom)
			{
				return current_floor;
			},
			[=](get_current_state_name_atom)
			{
				return fsm->get_state_name();
			},
			[=](get_elevator_number_atom)
			{
				return elevator_number;
			},
			[=](set_number_atom, int number)
			{
				elevator_number = number;
			},
			[=](timer_atom)
			{
				debug_msg("timer_atom received");
				return fsm->handle_timer(*this);
			},
			[=](elevator_subscribe_atom, std::string subscriber_key, elevator_observable_event_type event_type)
			{
				add_subscriber(current_sender(), subscriber_key, event_type);
				debug_msg("subscribe_atom received");

			}
		};
	}

	// set next state, calling on_exit and on_enter functions
	void elevator_actor::transition_to_state(std::shared_ptr<elevator_fsm> state)
	{
		if (this->fsm)
			this->fsm->on_exit(*this);
		this->fsm = state;
		this->fsm->on_enter(*this);
	}

	// cya
	void elevator_actor::on_quit()
	{
		anon_send_exit(this, exit_reason::user_shutdown);
	}

	// set up
	bool elevator_actor::on_initialise()
	{
		// transition to `unconnected` on elevator controller failure/shutdown
		// set the handler if we lose connection to elevator controller 
		set_down_handler([=](const down_msg& dm)
			{
				if (dm.source == controller)
				{
					debug_msg("lost connection to elevator controller, please reconnect or quit");
					this->controller = nullptr;
				}
			});

		return true;
	}

	// connect to the elevator controller
	void elevator_actor::on_connect(const std::string& host, uint16_t port)
	{
		// make sure we are not pointing to an old controller
		controller = nullptr;
		controller_host.assign("");
		controller_port = 0;

		// use request().await() to suspend regular behavior until MM responded
		auto mm = system().middleman().actor_handle();

		request(mm, infinite, connect_atom_v, host, port)
			.await
			(
				[host, port, this](const node_id&, strong_actor_ptr controller, const std::set<std::string>& ifs)
				{
					if (!controller)
					{
						debug_msg(R"(>>> no controller found at ")" + host + R"(":)" + std::to_string(port) + " <<<");
						return;
					}
					if (!ifs.empty())
					{
						debug_msg(R"(>>> typed actor found at ")" + host + R"(":)" + std::to_string(port) + ", but expected an untyped actor <<<");
						return;
					}
					debug_msg("successfully connected to controller");
					controller_host.assign(host);
					controller_port = port;
					this->controller = controller;
					auto controller_hdl = actor_cast<actor>(controller);
					this->monitor(controller_hdl);
					this->send(actor_cast<caf::actor>(controller), register_elevator_atom_v);
					transition_to_state(elevator_fsm::idle);
				},
				[host, port, this](const error& err)
				{
					debug_msg(R"(>>> cannot connect to ")" + host + R"(":)" + std::to_string(port) + " => " /*+ this->system().render(err)*/ + " <<<");
					transition_to_state(elevator_fsm::disconnected);
				}
				);
	}

	// waypoint floor received from controller
	void elevator_actor::on_waypoint_received(int waypoint_floor)
	{
		if (waypoint_floor > elevator::TOP_FLOOR || waypoint_floor < elevator::BOTTOM_FLOOR)
			return;
		waypoint_floors.push(waypoint_floor); // Note that elevator will visit floors in order received, usually ascending or descending
	}

	// no more waypoints/passengers, let the controller know and then wait for a job from the controller
	void elevator_actor::on_idle()
	{
		if(dispatcher)
			send(actor_cast<caf::actor>(dispatcher), elevator_idle_atom_v, elevator_number, current_floor);

	}

	// start the lift if there are any waypoints
	// return true: has waypoints and ready to go, false: no waypoints
	// FSM only has idle_state calling this; it's just needed to kick off state transitions from idle if there are waypoints
	bool elevator_actor::on_start()
	{
		return true;
	}

	// starting moving
	void elevator_actor::on_in_transit()
	{
	}

	// at a floor, doors opening, picking up & dropping off passengers, 
	void elevator_actor::on_waypoint_arrive()
	{
		// let the dispatcher know we've arrived, so it can inform the passenger(s)
		if(dispatcher)
			send(actor_cast<caf::actor>(dispatcher), waypoint_arrived_atom_v, elevator_number, current_floor);
	}

	// timer is used to simulate travel between floors or delay at waitpoint while passenger embark/disembark
	void elevator_actor::timer_pulse(int seconds)
	{
		delayed_send(this, std::chrono::seconds(seconds), timer_atom_v);
	}

	// send debug message to all subscribers, e.g. repl
	void elevator_actor::debug_msg(std::string msg)
	{
		string subscriber_msg = "[elevator][" + std::to_string(elevator_number) + "][" + fsm->get_state_name() + "][" + std::to_string(current_floor) + "]: " + msg;
		for (auto kv : debug_message_subscribers)
		{
			auto recipient = actor_cast<actor>(kv.second);
			send(recipient, message_atom_v, subscriber_msg);
		}

	}

	// register debug message subscriber, e.g. repl actor
	void elevator_actor::add_subscriber(strong_actor_ptr subscriber, std::string subscriber_key, elevator::elevator_observable_event_type event_type)
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
			//debug_message_subscribers.insert(std::make_pair<string, const actor&>(subscriber_key, subscriber));
			debug_message_subscribers[subscriber_key] = std::move(subscriber);
		}
		break;
		default:
			break;
		};
	}


}
