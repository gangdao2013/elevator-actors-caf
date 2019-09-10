# elevator-actors-caf: Elevator simulator using actors.

This is a demo project in C++ showing the C++ Actor Framework (CAF) being used to model elevators, passengers and controllers.

See more information about CAF here: http://www.actor-framework.org

The project is currently structured as a Visual Studio 2019 C++ solution, that relies on the CAF vcpkg being installed user-globally. At some stage I plan to move this over to CMake.

The project is using GitFlow workflow; see develop and feature branches for latest code.

If you don't have vcpkg installed, then just follow the directions here: https://docs.microsoft.com/en-us/cpp/build/vcpkg?view=vs-2019

Once you have vcpkg installed, run 

```sh
vcpkg install caf
vcpkg integrate install
```

The solution should build and run as-is. Check out main.cpp for details, but you can run up multiple instances of elevator.exe in either:

* Controller mode: the controller receives call requests from passengers, monitors and dispatches waypoints to elevators
* Passenger mode: makes calls, gets in and out of elevators
* Elevator mode: travels up and down, picking up and dropping off passengers

In summary:
```
elevator.exe -C
elevator.exe -P --name <passenger-name> --host <host> --port <port> (host/port default to localhost/10000)
elevator.exe -E --host <host> --port <port> (host/port default to localhost/10000)
```
Each mode presents a REPL that you can use to drive the underlying actor(s). The REPL is handy for driving, testing and debugging the application. For example, start up a controller (`elevator.exe -C`) on a command shell and then debug a passenger in Visual Studio, or you could even start up multiple instances of Visual Studio and debug in different modes.

Note that the application can be run across a network, e.g. a Passenger can connect to a Controller on a different machine. In this instance specify the host & port of the Controller when starting a Passenger or Elevator, through the --host and --port options. 

Modes in more detail:

Controller mode: 
----------------
Runs as the main controller/supervisor for the system. An embedded controller actor and linked controller REPL actor will start.
It doesn't (yet) monitor/respawn actors, but planned for the future. 

Usage:

> elevator.exe -C --port <port number, default 10000> 
...will start a controller on localhost on default port 10000 (you can change this in elevator/elevator.hpp). Run one controller per session.

Elevator mode: 
--------------
Runs as an elevator. An embedded elevator actor and linked elevator REPL will start.

Usage:
```
elevator.exec -E --host <host-name> --port <port number, default 10000>
```
...will start an elevator on localhost, connecting to your controller on host/port. If not specified host/port will be localhost/1000.

You can run multiple elevators per session, in separate command windows. Each elevator has default capacity of 2 passengers, 
change this in elevator/elevator.hpp. Future versions will have capacity be a command line option.

Passenger mode:
--------------
Runs as a passenger. An embedded passenger actor and linked passenger REPL will start.

Usage:
```
elevator.exec -P --name <passenger-name> --host <host-name> --port <port number, default 10000>
```
...will start an passenger on localhost, connecting to your controller on host/port. If not specified host/port will be localhost/1000.

You can run multiple passengers per session, in separate command windows. To have have passengers call for lifts, in the REPL just type 'c <floor_to>', 
e.g. 'c 6' - this will call for an elevator from the passenger's current floor to floor 6. 

Have an experiment with multiple passengers (in multiple command windows) calling for elevators (running in their own command windows.)
Each actor prints a detailed log of their events & actions.

### Solution Structure
The solution consists of three VS 2019 C++ projects:

elevator: (32 bit exe) The application code.
elevator_core: (32 bit static lib) Library project, containing headers and code for all the elevator actors, FSMs and supporting classes.
elevator_tests: (32 bit tests) VS 2019 Native C++ test library.

As mentioned above, the solution has a dependency on the CAF libraries, which I found easiest to install and integrate into VS 2019 via vcpkg. At some stage I'll shift over to a cross-platform CMake-based solution.
On the Windows platform, CAF also has a dependency on these libraries: ws2_32.lib;wsock32.lib;Iphlpapi.lib; - if you are ever doing your project, make sure you include them as additional libraries.

### Debugging the Solution
It's pretty to debug or step through the code in Visual Studio, for any of the modes. Just set the 'elevator' project as your start up project, and set the relevant mode command line options in the Debug section of the project properties dialog (e.g. -C).
Once you fire up elevator (in whatever mode) in VS, then you can start another elevator.exe in a different mode in a separate command window (or even a separate VS 2019 instance.)

### Actors and Finite State Machines
Controllers, Passengers and Elevators are all modelled and simulated using CAF actors that delegate received messages to an embedded finite state machine (FSM). The FSMs call back into their attached actors to perform specific actions. The FSM model is broadly based on the GOF State pattern, but using shared_ptrs to transition between states and overridden event handler functions. See the various _fsm and _actor classes and headers for details.

### Developing using CAF
CAF uses a lot of C++ template meta-programming (see the CAF docs & code for why). This will commonly lead to much frustration when you are starting programming with CAF, for example, a seemingly innocuous error in your CAF code (not reported in the editor) will result in a compiler error several levels deep in some obscure CAF header file. If this happens to you, don't rely on the error code reported in the Visual Studio error window - best place to start is by carefully tracing the compilation backwards through the output window until you come upon your source file immediately before all the CAF source; most likely that's where the problem will be.
Check for const refs, wrong actor types, wrong message tuple types, etc.

### License

See the [LICENSE](LICENSE.md) file for license rights and limitations (MIT + Nice lunch for me if you use the code in a commercial product. That's fair enough isn't it?).
