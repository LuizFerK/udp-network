# Network Router Simulation

A multi-threaded network simulation program that implements a distributed router system using UDP sockets. The program allows routers to send and receive messages through a network topology defined by configuration files.

## Features

- **Multi-threaded Architecture**: Uses pthreads for concurrent message handling
- **UDP Communication**: Router-to-router communication via UDP sockets
- **Thread-safe Queues**: Message queues with mutex and semaphore synchronization
- **Interactive Menu**: Command-line interface for sending messages
- **Configurable Topology**: Network topology defined in configuration files
- **Message Routing**: Support for data and control message types

## Project Structure

```
net/
├── main.c                    # Entry point
├── net/
│   ├── defs.h               # Data structures and constants
│   ├── setup.c/h            # Router initialization and configuration
│   ├── helpers.c/h          # Utility functions
│   └── modules/
│       ├── menu.c/h         # Interactive command interface
│       ├── sender.c/h       # Message sending thread
│       ├── receiver.c/h     # Message receiving thread
│       └── packet_handler.c/h # Message processing thread
├── config/
│   ├── router.config        # Router definitions
│   └── link.config         # Network topology
└── Makefile                # Build configuration
```

## Building

The project uses a simple Makefile for compilation:

```bash
make
```

This will compile all source files and create the `net.o` executable. The build process includes:
- C99 standard compliance
- Wall and Wextra warnings
- pthread library linking

To clean build artifacts:
```bash
make clean
```

## Configuration

### Router Configuration (`config/router.config`)
Defines available routers in the network:
```
<router_id> <port> <host>
```

Example:
```
1 25001 127.0.0.1
2 25002 127.0.0.1
3 25003 127.0.0.1
4 25004 127.0.0.1
```

### Link Configuration (`config/link.config`)
Defines network topology with weighted links:
```
<source_router> <destination_router> <weight>
```

Example:
```
1 2 3
2 3 2
2 4 10
3 4 3
```

## Running

Start a router instance by specifying its ID:

```bash
./net.o -i <router_id>
```

Where `router_id` is an integer between 1 and 4 (based on the configuration).

Example:
```bash
./net.o -i 1
```

## Usage

Once the program starts, you'll see the router's network connections and an interactive menu:

```
Connected to Router 1: Host: 127.0.0.1, Port: 25001

-> Linked to Router 2: Host: 127.0.0.1, Port: 25002, Weight: 3

Available commands:
menu - show this help
send -t <destination> -m '<message>' - send a message
exit - exit the program
```

### Commands

- **menu**: Display available commands
- **send -t <destination> -m '<message>'**: Send a message to another router
- **exit**: Terminate the program

### Sending Messages

To send a message to another router:
```
send -t 2 -m 'Hello from router 1'
```

- `-t <destination>`: Target router ID
- `-m '<message>'`: Message content (supports quoted strings with spaces)

## Architecture

The program implements a three-thread architecture:

1. **Sender Thread**: Handles outgoing messages from the message queue
2. **Receiver Thread**: Listens for incoming UDP packets
3. **Packet Handler Thread**: Processes received messages

### Thread Communication

- **Sender Queue**: Thread-safe queue for outgoing messages
- **Packet Handler Queue**: Thread-safe queue for incoming messages
- **Mutexes**: Protect queue operations
- **Semaphores**: Signal when messages are available

### Message Structure

```c
typedef struct Message {
    int type;           // 1 for data, 2 for control
    int source;         // Source router ID
    int destination;    // Destination router ID
    char payload[100];  // Message content
} Message;
```

## Requirements

- GCC compiler with C99 support
- pthread library
- Linux/Unix environment with UDP socket support

## Example Session

1. Start router 1: `./net.o -i 1`
2. Start router 2: `./net.o -i 2` (in another terminal)
3. From router 1, send a message: `send -t 2 -m 'Test message'`
4. Router 2 will receive and display the message
5. Use `exit` to terminate either router
