# Mini-OS-Railway-Simulator

A command-line based Operating System simulator built in C++ that demonstrates the interaction between Kernel-level resource management and User-level application processes.

## Features
* **Kernel & User Modes:** Secure access control using password-protected Kernel Mode.
* **Process Management:** Simulates processes with custom RAM and CPU requirements.
* **Scheduling Algorithms:** * First-Come-First-Served (FCFS)
    * Round Robin (RR)
    * Priority Scheduling
* **Concurrency Control:** Uses `mutex` and `atomic` variables to handle thread-safe railway ticket booking and seat allocation.
* **Interrupt Handling:** Simulated interrupt mechanism for ticket cancellations.
* **System Logging:** Real-time logging of all system activities to `system_log.txt`.

## Technical Stack
* **Language:** C++11/17
* **Concurrency:** `<thread>`, `<mutex>`, `<atomic>`
* **Synchronization:** POSIX/Standard C++ Synchronization primitives

## How to Run
1. Ensure you have a C++ compiler installed (e.g., `g++`).
2. Compile the project:
   ```bash
   g++ -o os_sim main.cpp -lpthread
   ./os_sim

## Usage   
User Mode: Book tickets, view seat layouts, and inquire about status.
Kernel Mode (Password: admin123): Manage system processes, execute scheduling algorithms, and view internal system logs.
