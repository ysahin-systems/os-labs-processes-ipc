# time_shm

Simple Linux utility that measures how long a command takes to execute,
using **POSIX shared memory** for IPC between parent and child.

## How it works

- Parent process creates a shared memory object (`shm_open`, `mmap`).
- Child records the start time with `gettimeofday()` into shared memory,
  then executes the given command via `execvp`.
- Parent waits for the child (`wait`), takes the end time, and
  computes the elapsed time using the shared timestamp.

## Usage

```bash
./time_shm ls
./time_shm sleep 3

