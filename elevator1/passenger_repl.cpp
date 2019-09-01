#include "passenger_repl.hpp"

#include <iostream>
#include <vector>
#include <string>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "string_util.hpp"

#include "elevator.hpp"
#include "controller.hpp"
#include "passenger.hpp"

using namespace caf;
using namespace std;
using namespace elevator;

namespace passenger
{

	behavior passenger_repl::make_behavior()
	{
		return
		{
			[&](elevator::get_instructions_atom, int current_floor) -> message {return next_instruction(current_floor); }
		};
	}


	void passenger_usage() {
		cout << "Usage:" << endl
			<< "  quit                  : terminates the program\n" 
			<< "  connect <host> <port> : connects to a (remote) lift controller\n" 
			<< "  f <to>                : call elevator from current floor to floor <to> (0 is ground floor)"s << std::endl
			;
	};

	message passenger_repl::next_instruction(int floor)
	{


		//auto current_floor = [&]() -> int {
		//	auto self = scoped_actor { system };
		//	self->request(passenger, infinite, elevator::get_current_passenger_floor_atom::value).receive(
		//		[&](int floor) {return floor; },
		//		[&](error& err) { aout(self) << "error: " << self->system().render(err) << endl; return 0; }
		//	);
		//};


		bool done = false;

		// defining the handler outside the loop is more efficient as it avoids
		// re-creating the same object over and over again
		message_handler eval
		{
			[&](const string& cmd) -> caf::optional<message>
			{
				if (cmd != "quit")
					return {};
				done = true;
				return make_message(quit_atom::value);
			},
			[&](string& arg0, string& arg1, string& arg2) -> caf::optional<message>
			{
				if (arg0 == "connect")
				{
					char* end = nullptr;
					auto lport = strtoul(arg2.c_str(), &end, 10);
					if (end != arg2.c_str() + arg2.size())
					{
						cout << R"(")" << arg2 << R"(" is not an unsigned integer)" << endl;
						return {};
					}
					else if (lport > std::numeric_limits<uint16_t>::max())
					{
						cout << R"(")" << arg2 << R"(" > )" << std::numeric_limits<uint16_t>::max() << endl;
						return {};
					}
					else
						return make_message(connect_to_controller_atom::value, arg1, lport);
				}
				return {};
			},
			[&](string& arg0, string& arg1) -> caf::optional<message>
			{
				if (arg0 == "f")
				{
					auto to_floor = string_util::to_integer(arg1);
					if (to_floor.has_value())
					{
						//self->state.called_floor = to_floor; // .value();
						//cout << "\nCalling elevator from floor: " << self->state.current_floor << " to floor: "; // << to_floor.value() << endl;
						//self->send(self->state.current_controller, elevator::call_atom::value, self, self->state.current_floor, self->state.called_floor);
						//self->become(waiting_for_elevator);
						return make_message(call_atom::value, to_floor.value());
					}
				}
				return {};
			}
		};

		string line;
		//int floor = current_floor();
		cout << "\nf " << floor << "> " << endl;
		while (!done)
		{
			std::getline(std::cin, line);
			line = string_util::trim(std::move(line)); // ignore leading and trailing whitespaces
			std::vector<string> words;
			split(words, line, is_any_of(" "), token_compress_on);

			auto result = message_builder(words.begin(), words.end()).apply(eval);

			if (!result || result.value().size() == 0)
				passenger_usage();
			else
				return result.value();
			cout << "\nf " << floor << "> " << endl;
		}
	}
}