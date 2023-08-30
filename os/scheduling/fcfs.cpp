#include <algorithm>
#include <iostream>
#include <vector>

// Process structure
struct Process {
    int pid;
    int arrivalTime;
    int executionTime;
    int startingTime;
    int finishedTime;
    int waitingTime;
    int turnAroundTime;
};

// Method to compare two proces for sorting purpose.
bool processCompare(Process& p1, Process& p2) {
    if (p1.arrivalTime == p2.arrivalTime) {
        return p1.pid < p2.pid;
    }
    return p1.arrivalTime < p2.arrivalTime;
}

class FCFS {
  private:
    int mTimer;
    std::vector<Process> mProcessQueue;

  public:
    FCFS(std::vector<Process>&);
    void process();
    void print();
    void printGanttChart();
};

// FCFS constructor
FCFS::FCFS(std::vector<Process>& processes) : mTimer{0}, mProcessQueue{processes} {
    // Sort the processes based on arrival time and pid.
    std::sort(mProcessQueue.begin(), mProcessQueue.end(), processCompare);
}

void FCFS::process() {
    for (auto& process : mProcessQueue) {
        // Update timer if it is running behing arrival of current process.
        if (mTimer < process.arrivalTime)
            mTimer = process.arrivalTime;

        process.startingTime = mTimer; // Start the process
        mTimer += process.executionTime;
        process.finishedTime = mTimer; // Stop the process

        // Turn around time.
        process.turnAroundTime = process.finishedTime - process.arrivalTime;
        // Waiting time.
        process.waitingTime = process.turnAroundTime - process.executionTime;
    }
}

void FCFS::print() {
    process();
    std::cout << "P\tAT\tBT\tST\tFT\tWT\tTAT\n";

    int waitingTime = 0;
    int turnAroundTime = 0;
    for (auto& process : mProcessQueue) {
        std::cout << process.pid << '\t';
        std::cout << process.arrivalTime << '\t';
        std::cout << process.executionTime << '\t';
        std::cout << process.startingTime << '\t';
        std::cout << process.finishedTime << '\t';
        std::cout << process.waitingTime << '\t';
        std::cout << process.turnAroundTime << '\n';

        waitingTime += process.waitingTime;
        turnAroundTime += process.turnAroundTime;
    }
    std::cout << std::endl;

    std::cout << "Average waiting time: " << double(waitingTime) / mProcessQueue.size() << std::endl;
    std::cout << "Average turn around time: " << double(turnAroundTime) / mProcessQueue.size() << std::endl;
    std::cout << std::endl;

    printGanttChart();
}

void FCFS::printGanttChart() {
    std::cout << "Process: ";
    for (Process& process : mProcessQueue) {
        std::cout << process.pid << '\t';
    }
    std::cout << std::endl;

    std::cout << "Time   : ";
    for (Process& process : mProcessQueue) {
        std::cout << process.finishedTime << '\t';
    }
    std::cout << std::endl;
}

//----------------------------------------------------------------
// Main function
//----------------------------------------------------------------
int main(int argc, char* argv[]) {
    std::vector<Process> processList = {
        // pid, arrival, execution
        {0,  0, 5},
        {1,  4, 3},
        {2,  3, 3},
        {3, 12, 4},
    };

    FCFS(processList).print();

    return 0;
}