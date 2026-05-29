#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <fstream>
#include <stdexcept>
#include <atomic>
#include <algorithm>
#include <iomanip>

using namespace std;

// ANSI Color Codes
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

struct Process
{
    int pid;
    string state; // New, Ready, Running, Blocked, Terminated
    int memoryRequired;
    int cpuBurst;
    int priority;
    string passengerName;
    int arrivalTime;
};

int TOTAL_RAM = 2048;
int TOTAL_CORES = 8;
int availableRAM;
int availableCores;
int systemClock = 0;

queue<Process> readyQueue;
vector<Process> blockedQueue;
mutex queueMutex;
mutex seatMutex;

// Railway Data
const int totalSeats = 20;
atomic<int> availableSeats(20);
bool seats[totalSeats + 1] = {false};
vector<string> passengerRecords;

ofstream logFile;

int getSafeInt()
{
    int x;
    while (!(cin >> x))
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << RED << "Invalid input. Please enter a number: " << RESET;
    }
    return x;
}

/**
 * Function: logActivity
 * Description: Maintains system activity logs in system_log.txt[cite: 148].
 */
void logActivity(const string &msg)
{
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    char buf[26];
    ctime_r(&t, buf);
    string timestamp(buf);
    timestamp.pop_back();
    logFile << "[" << timestamp << "] " << msg << endl;
}

/**
 * Function: bootAnimation
 */
void bootAnimation()
{
    cout << CYAN << "Kernel Initializing..." << RESET << endl;
    for (int i = 0; i <= 100; i += 20)
    {
        cout << "[";
        for (int j = 0; j < 10; j++)
        {
            if (j < i / 10)
                cout << GREEN << "=" << RESET;
            else
                cout << " ";
        }
        cout << "] " << i << "%" << "\r";
        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(300));
    }
    cout << endl;
}

// ==================== OS Core Management ====================

/**
 * Function: bootSystem
 */
void bootSystem()
{
    bootAnimation();
    availableRAM = TOTAL_RAM;
    availableCores = TOTAL_CORES;
    availableSeats = 20;
    systemClock = 0;
    for (int i = 0; i <= totalSeats; i++)
        seats[i] = false;
    logActivity("System booted successfully.");
    cout << GREEN << "System Booted. Resources Initialized." << RESET << endl;
}

bool createProcess(int pid, int memReq, int burst, int prio, string name)
{
    if (memReq > availableRAM || availableCores <= 0)
    {
        cout << RED << "Process Creation Failed: Insufficient Resources." << RESET << endl;
        return false;
    }

    // Transition: New -> Ready
    availableRAM -= memReq;
    availableCores--;
    Process p = {pid, "Ready", memReq, burst, prio, name, systemClock};

    {
        lock_guard<mutex> lock(queueMutex);
        readyQueue.push(p);
    }
    logActivity("Process " + to_string(pid) + " moved to READY state.");
    return true;
}

void handleInterrupt(string type, int pid)
{
    cout << MAGENTA << "\n[INTERRUPT] " << type << " signal received for PID " << pid << RESET << endl;
    logActivity("Interrupt: " + type + " for PID " + to_string(pid));
    // Implementation of blocking logic
}

void terminateProcess(int pid, int memUsed)
{
    availableRAM += memUsed;
    availableCores = min(availableCores + 1, TOTAL_CORES);
    cout << YELLOW << "Process " << pid << " [TERMINATED]. Resources recovered." << RESET << endl;
}

void fcfsScheduler()
{
    cout << BLUE << "\n--- Running FCFS Scheduler ---" << RESET << endl;
    while (!readyQueue.empty())
    {
        Process p;
        {
            lock_guard<mutex> lock(queueMutex);
            p = readyQueue.front();
            readyQueue.pop();
        }

        p.state = "Running";
        cout << "PID " << p.pid << " is " << GREEN << "RUNNING" << RESET << endl;
        this_thread::sleep_for(chrono::milliseconds(p.cpuBurst * 50));

        systemClock += p.cpuBurst;
        terminateProcess(p.pid, p.memoryRequired);
    }
}

void roundRobinScheduler(int quantum = 4)
{
    cout << BLUE << "\n--- Running Round Robin (Quantum: " << quantum << ") ---" << RESET << endl;
    while (!readyQueue.empty())
    {
        Process p;
        {
            lock_guard<mutex> lock(queueMutex);
            p = readyQueue.front();
            readyQueue.pop();
        }

        p.state = "Running";
        int runTime = min(quantum, p.cpuBurst);
        cout << "PID " << p.pid << " Running for " << runTime << " units." << endl;
        this_thread::sleep_for(chrono::milliseconds(runTime * 50));

        p.cpuBurst -= runTime;
        systemClock += runTime;

        if (p.cpuBurst > 0)
        {
            p.state = "Ready";
            lock_guard<mutex> lock(queueMutex);
            readyQueue.push(p);
        }
        else
        {
            terminateProcess(p.pid, p.memoryRequired);
        }
    }
}

void priorityScheduler()
{
    cout << BLUE << "\n--- Running Priority Scheduler ---" << RESET << endl;
    vector<Process> pList;
    {
        lock_guard<mutex> lock(queueMutex);
        while (!readyQueue.empty())
        {
            pList.push_back(readyQueue.front());
            readyQueue.pop();
        }
    }
    sort(pList.begin(), pList.end(), [](const Process &a, const Process &b)
         { return a.priority > b.priority; });

    for (auto &p : pList)
    {
        cout << "PID " << p.pid << " [Priority " << p.priority << "] is Running." << endl;
        this_thread::sleep_for(chrono::milliseconds(p.cpuBurst * 50));
        terminateProcess(p.pid, p.memoryRequired);
    }
}

void displaySeatMap()
{
    lock_guard<mutex> lock(seatMutex);
    cout << CYAN << "\nRailway Seat Layout:" << RESET << endl;
    for (int i = 1; i <= totalSeats; i++)
    {
        cout << "[" << (i < 10 ? "0" : "") << i << (seats[i] ? RED " X" RESET : "  ") << "]  ";
        if (i % 4 == 0)
            cout << endl;
    }
}

bool bookTicket(int pid, string name)
{
    lock_guard<mutex> lock(seatMutex);
    if (availableSeats <= 0)
        return false;

    cout << "Select: 1. Window  2. Aisle: ";
    int type = getSafeInt();
    int assigned = -1;

    for (int i = 1; i <= totalSeats; i++)
    {
        if (!seats[i])
        {
            bool isWin = (i % 4 == 1 || i % 4 == 0);
            if ((type == 1 && isWin) || (type == 2 && !isWin))
            {
                assigned = i;
                break;
            }
        }
    }

    if (assigned != -1)
    {
        seats[assigned] = true;
        availableSeats--;
        passengerRecords.push_back("PID: " + to_string(pid) + " | Name: " + name + " | Seat: " + to_string(assigned));
        cout << GREEN << "Seat #" << assigned << " Booked Successfully!" << RESET << endl;
        return true;
    }
    return false;
}

void cancelTicket()
{
    cout << "Enter Seat Number to Cancel: ";
    int sn = getSafeInt();
    lock_guard<mutex> lock(seatMutex);
    if (sn > 0 && sn <= totalSeats && seats[sn])
    {
        seats[sn] = false;
        availableSeats++;
        handleInterrupt("Ticket Cancellation", 0);
        cout << GREEN << "Ticket Cancelled." << RESET << endl;
    }
    else
    {
        cout << RED << "Seat not found or not booked." << RESET << endl;
    }
}

void userModeMenu()
{
    static int uPid = 1000;
    while (true)
    {
        cout << GREEN << "\n--- USER MODE ---" << RESET << endl;
        cout << "1. View Seats\n2. Book Ticket\n3. Inquiry\n4. Exit\nChoice: ";
        int c = getSafeInt();
        if (c == 1)
            displaySeatMap();
        else if (c == 2)
        {
            string n;
            cout << "Name: ";
            cin.ignore();
            getline(cin, n);
            if (createProcess(uPid, 100, 5, 2, n))
            {
                if (bookTicket(uPid, n))
                    uPid++;
            }
        }
        else if (c == 3)
        {
            for (auto &r : passengerRecords)
                cout << r << endl;
        }
        else if (c == 4)
            break;
    }
}

void kernelModeMenu()
{
    cout << RED << "Admin Password: " << RESET;
    string p;
    cin >> p;
    if (p != "admin123")
    {
        logActivity("Unauthorized access attempt.");
        return;
    }

    while (true)
    {
        cout << YELLOW << "\n--- KERNEL MODE (Privileged) ---" << RESET << endl;
        cout << "1. System Status\n2. Run FCFS\n3. Run Round Robin\n4. Run Priority\n5. Cancel Ticket (Interrupt)\n6. View Logs\n7. Exit\nChoice: ";
        int c = getSafeInt();
        if (c == 1)
        {
            cout << "RAM: " << availableRAM << " Cores: " << availableCores << endl;
            displaySeatMap();
        }
        else if (c == 2)
            fcfsScheduler();
        else if (c == 3)
            roundRobinScheduler();
        else if (c == 4)
            priorityScheduler();
        else if (c == 5)
            cancelTicket();
        else if (c == 6)
        {
            logFile.flush();
            ifstream f("system_log.txt");
            string l;
            while (getline(f, l))
                cout << l << endl;
        }
        else if (c == 7)
            break;
    }
}

int main()
{
    logFile.open("system_log.txt", ios::app);
    bootSystem();

    while (true)
    {
        cout << CYAN << "\n======================================" << RESET << endl;
        cout << CYAN << "      MINI OPERATING SYSTEM V2.0      " << RESET << endl;
        cout << CYAN << "======================================" << RESET << endl;
        cout << "1. User Mode\n2. Kernel Mode\n3. Shutdown\nSelect: ";
        int m = getSafeInt();

        if (m == 1)
            userModeMenu();
        else if (m == 2)
            kernelModeMenu();
        else if (m == 3)
        {
            logActivity("System shutting down.");
            cout << RED << "Shutting down Mini OS..." << RESET << endl;
            break;
        }
    }
    logFile.close();
    return 0;
}
