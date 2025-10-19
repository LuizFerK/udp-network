# Network Router Simulation

A multi-threaded network simulation that implements distributed routing using the Bellman-Ford distance vector algorithm. Routers communicate via UDP and automatically discover optimal routes through periodic distance vector exchanges.

## Features

- **Distance Vector Routing**: Automatic route discovery using Bellman-Ford algorithm
- **Multi-threaded Architecture**: Concurrent message handling with pthreads
- **UDP Communication**: Router-to-router communication via UDP sockets
- **Interactive Interface**: ncurses-based UI for sending messages and monitoring status
- **Thread-safe Queues**: Synchronized message queues with mutexes and semaphores
- **Link Monitoring**: Automatic timeout detection and route recovery

## Architecture

**Four-thread system:**
- **Sender Thread**: Processes outgoing messages from queue
- **Receiver Thread**: Listens for incoming UDP packets  
- **Packet Handler Thread**: Routes messages and handles distance vectors
- **Routing Thread**: Periodically updates routing table and sends distance vectors to neighbors

**Routing Algorithm:**
- Bellman-Ford distance vector algorithm
- Periodic distance vector updates to neighbors
- Automatic route convergence and link timeout detection

## Building

```bash
make                    # Compile the project
make clean             # Remove build artifacts
```

**Requirements:**
- GCC with C99 support
- Linux/Unix environment

## Configuration

**Router Config** (`config/router.config`):
```
<router_id> <port> <host>
1 25001 127.0.0.1
2 25002 127.0.0.1
3 25003 127.0.0.1
4 25004 127.0.0.1
```

**Link Config** (`config/link.config`):
```
<source> <destination> <weight>
1 2 3
2 3 2
2 4 10
3 4 3
```

## Usage

**Start a router:**
```bash
./net.o -i <router_id> [-t <routing_timeout_seconds>]
```

**Commands:**
- `send -t <destination> -m '<message>'` - Send message to another router
- `status` - Show routing table and distance vectors
- `exit` - Terminate the program

**Example:**
```bash
# Terminal 1
./net.o -i 1

# Terminal 2  
./net.o -i 2

# From router 1, send a message
send -t 2 -m 'Hello from router 1'
```
