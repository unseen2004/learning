# C++ Concurrent Programming Projects

This directory contains C++ projects related to concurrent programming.

## Programs

### `costumer_service.cpp`

A simulation of a customer service center with multiple service desks and a shared customer queue. This program demonstrates the use of `std::thread`, `std::mutex`, and `std::condition_variable` for thread synchronization.

#### How to Run
```bash
g++ -std=c++17 -pthread costumer_service.cpp -o costumer_service
./costumer_service
```

### `parallel_transform.cpp`

An implementation of a parallel `std::transform` algorithm. It divides the work of applying a function over a range of elements among multiple threads.

#### How to Run
```bash
g++ -std=c++17 -pthread parallel_transform.cpp -o parallel_transform
./parallel_transform
```

### `thread_qs.cpp`

A parallel implementation of the Quicksort algorithm using `std::async` to sort sub-arrays concurrently.

#### How to Run
```bash
g++ -std=c++17 -pthread thread_qs.cpp -o thread_qs
./thread_qs
```

## Projects

### `game_of_life`

A parallel implementation of Conway's Game of Life using SDL2 for visualization. The game state is updated in parallel by multiple threads.

#### How to Run

1.  **Prerequisites:** You need to have `SDL2` and `SDL2_ttf` development libraries installed.
2.  **Build:**
    ```bash
    cd game_of_life
    mkdir -p build
    cd build
    cmake ..
    make
    ```
3.  **Run:**
    ```bash
    ./GameOfLife
    ```
Controls:
*   `q`: Quit the application.
*   `w`: Increase speed.
*   `s`: Decrease speed.
