# elevator-actors-caf: Elevator simulator using actors.

`Note: Under construction (not completed); actors currently being wired up - see develop branch`

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

Respectively:
```
elevator.exe -C
elevator.exe -P
elevator.exe -E
```
Each mode presents a REPL that you can use to drive the underlying actor(s). The REPL is handy for driving, testing and debugging the application. For example, start up a controller (`elevator.exe -C`) on a command shell and then debug a passenger in Visual Studio, or you could even start up multiple instances of Visual Studio and debug in different modes.

Note that the application can be run across a network, e.g. a Passenger can connect to a Controller on a different machine. In this instance specify the host & port of the Controller when starting a Passenger or Elevator, through the --host and --port options. 

E.g.:
```
elevator.exe -P --host my_controller_host --port 4500
```
...will connect a Passenger to a controller on 'my_controller_host' on port 4500.

### Actors and Finite State Machines
Controllers, Passengers and Elevators are all modelled and simulated using CAF actors that delegate received messages to an embedded finite state machine (FSM). The FSMs call back into their attached actors to perform specific actions. The FSM model is broadly based on the GOF State pattern, but using shared_ptrs to transition between states and overridden event handler functions. See the various _fsm and _actor classes and headers for details.

### Developing using CAF
CAF uses a lot of C++ template meta-programming (see the CAF docs & code for why). This will commonly lead to much frustration when you are starting programming with CAF, for example, a seemingly innocuous error in your CAF code (not reported in the error) will result in a compiler error several levels deep in some obscure CAF header file. If this happens to you, don't rely on the error code reported in the Visual Studio error window - best place to start is by carefully tracing the compilation backwards through the output window until you come upon your source file immediately before all the CAF source; most likely that's where the problem will be.
Check for const refs, wrong actor types, wrong message tuple types, etc.
