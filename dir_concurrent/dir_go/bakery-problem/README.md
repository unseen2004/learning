# Mutex Algorithm Implementations in Go

This directory contains Go implementations of various algorithms for mutual exclusion.

## Algorithms

*   `bakery_mutex.go`: Lamport's Bakery Algorithm.
*   `dekker_mutex_4.go`: Dekker's Algorithm (for 4 processes).
*   `peterson_mutex_6.go`: Peterson's Algorithm (for 6 processes).

## How to Run

To run any of the algorithm simulations, use the `go run` command:

```bash
go run bakery_mutex.go
# or
go run dekker_mutex_4.go
# or
go run peterson_mutex_6.go
```

The output is a trace of the simulation that can be visualized using the provided `display-travel.bash` script.

## Visualization

The `display-travel.bash` script can be used to visualize the execution trace of the algorithms. It requires a tool that can process the output format, which is not included in this repository.

```bash
go run bakery_mutex.go | ./display-travel.bash
```
