#include <algorithm>
#include <iostream>
#include <queue>
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

// Comparator for priority queue.
class ProcessComparator {
  public:
    bool operator()(const Process& p1, const Process& p2) {
        if (p1.executionTime == p2.executionTime) {
            return p1.pid > p2.pid;
        }
        return p1.executionTime > p2.executionTime;
    }
};

class SJF {
  private:
    int mTimer;
    std::priority_queue<Process, std::vector<Process>, ProcessComparator> mRunningProcess;
    std::queue<Process> mProcessList;
    std::vector<Process> mFinishedProcesses;

  public:
    SJF(std::vector<Process>&);
    void scheduleProcess();
    void print();
    void printGanttChart();
};

// SJF Constructor
SJF::SJF(std::vector<Process>& processes) : mTimer(0) {
    // Sort process list
    std::sort(processes.begin(), processes.end(), processCompare);

    // Load all processes in queue.
    for (Process p : processes)
        mProcessList.push(p);
}

void SJF::scheduleProcess() {
    while (!mRunningProcess.empty() || !mProcessList.empty()) {
        // If Running queue is empty the load a process from the queue.
        if (mRunningProcess.empty()) {
            mRunningProcess.push(mProcessList.front());
            mProcessList.pop();
        }

        // If timer is behind the top process, then update timer.
        if (mTimer < mRunningProcess.top().arrivalTime) {
            mTimer = mRunningProcess.top().arrivalTime;
        }

        // Load all the processes into running queue whose arrival
        // is less than current timer.
        while (!mProcessList.empty() && mProcessList.front().arrivalTime <= mTimer) {
            mRunningProcess.push(mProcessList.front());
            mProcessList.pop();
        }

        // Load current process
        Process currentProcess = mRunningProcess.top();
        mRunningProcess.pop();

        // Perform execution
        currentProcess.startingTime = mTimer;
        currentProcess.finishedTime = mTimer + currentProcess.executionTime;
        mTimer = currentProcess.finishedTime;

        // Send finished process to finished list.
        mFinishedProcesses.push_back(currentProcess);
    }

    for (Process& p : mFinishedProcesses) {
        p.turnAroundTime = p.finishedTime - p.arrivalTime;
        p.waitingTime = p.turnAroundTime - p.executionTime;
    }
}

void SJF::print() {
    scheduleProcess();
    std::cout << "P\tAT\tBT\tST\tFT\tWT\tTAT\n";

    int waitingTime = 0;
    int turnAroundTime = 0;
    for (auto& process : mFinishedProcesses) {
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

    std::cout << "Average waiting time: " << double(waitingTime) / mFinishedProcesses.size() << std::endl;
    std::cout << "Average turn around time: " << double(turnAroundTime) / mFinishedProcesses.size() << std::endl;
    std::cout << std::endl;

    printGanttChart();
}

void SJF::printGanttChart() {
    std::cout << "Process: ";
    for (Process& process : mFinishedProcesses) {
        std::cout << process.pid << '\t';
    }
    std::cout << std::endl;

    std::cout << "Time   : ";
    for (Process& process : mFinishedProcesses) {
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
        {0,  2, 5},
        {1,  4, 3},
        {2,  3, 2},
        {3, 12, 4},
    };

    SJF(processList).print();

    return 0;
}
