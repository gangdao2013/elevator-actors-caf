#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include <iostream>
#include <functional>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace caf;


namespace elevator
{
	class config : public actor_system_config {
	public:
		uint16_t port = 10000;
		std::string host = "localhost";
		bool controller_mode = false;

		config() {
			opt_group{ custom_options_, "global" }
				.add(port, "port,p", "set port")
				.add(host, "host,H", "set host (ignored in controller mode)")
				.add(controller_mode, "controller-mode,c", "enable controller mode");
		}
	};
}