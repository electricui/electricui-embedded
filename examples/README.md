# Example Code and Projects

A large chunk of the examples folder are specifically Arduino sketches, as the generic embedded ecosystem is pretty wide.  
Files in the `standalone` directory are not intended for Arduino users, targetted at desktop users and 'bare-metal' use.

Most of the Arduino example programs should be easily adaptable to any other generic C or C++ program.

# Arduino Examples

Examples in the basics, intermediate and project folders are intended for most arduino compatible boards. 

Some more complicated examples, such as the websockets and bluetooth transports in `intermediate` will require supporting hardware, and might need some minor tweaks depending on compatibility with any 3rd party libraries.

Project examples cover some pretty common hardware, again, supporting hardware is required for unmodified programs to run.

# Standalone Examples

A series of very small C examples are provided to demonstrate how to use the library with straight C.

These examples assume use of standard C tooling, `make` and `gcc` in particular.  
They have been developed and tested against a x86 GNU/Linux system, though they should work fine on ARM architectures and probably Windows as well.

In all cases, run `make` from the given example directory and then execute the program.

## Named Pipes

Run a small program which provides the output of a few variables to stdout, and uses named pipes for communication with the user interface.



## Websockets

Minimal example using $WS_LIBRARY_OF_CHOICE. Mimics the Arduino `intermediate/transport-websocket` example.

One packet per inbound/outboudn websocket transaction, in binary.

