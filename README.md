# Network Router Simulation

A multi-threaded network simulation program that implements a distributed router system using UDP sockets. The program allows routers to send and receive messages through a network topology defined by configuration files.

## Features

- **Multi-threaded Architecture**: Uses pthreads for concurrent message handling
- **UDP Communication**: Router-to-router communication via UDP sockets
- **Thread-safe Queues**: Message queues with mutex and semaphore synchronization
- **Interactive Menu**: Command-line interface for sending messages
- **Configurable Topology**: Network topology defined in configuration files
- **Distance Vector Routing**: Implements distributed routing algorithm with automatic route discovery
- **Routing Table**: Dynamic routing table management with next-hop information
- **Link State Monitoring**: Automatic link timeout detection and route updates
- **Message Routing**: Support for data and control message types

## Routing Behavior

### Automatic Route Discovery

The system automatically discovers routes through distance vector updates:

- **Periodic Updates**: Each router broadcasts its distance vector every 30 seconds (or custom timeout)
- **Route Learning**: Routers learn about network topology from received distance vectors
- **Convergence**: Routes automatically converge to optimal paths as updates propagate

### Link Monitoring

- **Timeout Detection**: Links are marked as expired if no updates are received within 3× timeout period
- **Automatic Recovery**: Routes are restored when connectivity is re-established
- **Route Updates**: Routing tables are updated when better paths are discovered

### Message Forwarding

- **Next-Hop Routing**: Messages include `next_hop` field for proper forwarding
- **Hop Counting**: Messages track the number of hops taken
- **Route Optimization**: Messages follow the shortest known path to destination

## Project Structure

```
net/
├── main.c                          # Entry point
├── net/
│   ├── defs.h                      # Data structures and constants
│   ├── setup.c/h                   # Router initialization and configuration
│   ├── helpers.c/h                 # Utility functions
│   └── modules/
│       ├── menu.c/h                # Interactive command interface
│       ├── sender.c/h              # Message sending thread
│       ├── receiver.c/h            # Message receiving thread
│       ├── packet_handler.c/h      # Message processing thread
│       └── routing.c/h             # Distance vector routing algorithm
├── config/
│   ├── router.config               # Router definitions
│   └── link.config                 # Network topology
└── Makefile                        # Build configuration
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

Start a router instance by specifying its ID and optional routing timeout:

```bash
./net.o -i <router_id> [-t <timeout_seconds>]
```

**Parameters:**
- `-i <router_id>`: Router ID (integer between 1 and 4)
- `-t <timeout_seconds>`: Routing update interval in seconds (default: 10)

**Examples:**
```bash
./net.o -i 1                    # Start router 1 with default 30s timeout
./net.o -i 2 -t 10              # Start router 2 with 10s update interval
```

## Usage

Once the program starts, you'll see the router's network connections, routing table, and an interactive menu:

```
Connected to Router 1: Host: 127.0.0.1, Port: 25001

-> Linked to Router 2: Host: 127.0.0.1, Port: 25002, Weight: 3

Routing table: 1 2 -1 -1 

[Sender] Waiting for messages to send...
[Receiver] Waiting for messages on port 25001...
[Packet Handler] Waiting for messages to handle...

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
    int next_hop;       // Next hop router for routing
    int hops;           // Number of hops taken
    char payload[100];  // Message content (or distance vector for control messages)
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
