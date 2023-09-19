#include <algorithm>
#include <deque>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>

// Process structure
struct Process {
    int pid;
    int arrivalTime;
    int priority;
    int burstTime;
    int tempBurstTime;
    int startingTime;
    int finishingTime;
    int turnAroundTime;
    int waitingTime;
    bool init;
};

class PriorityCompare {
  public:
    bool operator()(Process& a, Process& b) {
        if (a.priority == b.priority)
            return a.pid > b.pid;
        return a.priority > b.priority;
    }
};

bool compare(Process& p1, Process& p2) {
    return p1.arrivalTime < p2.arrivalTime;
}

// Class to hold all running processes and provide some
// useful functionalities.
class RunningProcess {
  private:
    // Index of the vector is priority.
    std::vector<std::queue<Process>> mProcessList;
    int processCount;

  public:
    RunningProcess() : processCount(0) {}

    int getSize() {
        return processCount;
    }

    bool empty() {
        return processCount == 0;
    }

    int addProcess(Process& process) {
        while (process.priority >= mProcessList.size()) {
            // If size of vector is less than the process.priority
            // then increase vector size.
            mProcessList.push_back(std::queue<Process>());
        }

        // Add the process to the list.
        mProcessList[process.priority].push(process);
        return ++processCount;
    }

    // Returns the higest priority process.
    Process getProcess() {
        for (auto& processList : mProcessList) {
            if (processList.size() > 0) {
                Process& process = processList.front();
                processList.pop();

                --processCount;
                return process;
            }
        }

        return Process(); // Return empty process.
    }
};

class PriorityPreemptive {
  private:
    std::queue<Process> mProcessList; // Initially store all processes in queue.
    // Store running processes.
    RunningProcess mRunningProcessList;
    std::vector<std::pair<int, int>> mGantChart; // <pid, time>
    std::vector<Process> mFinishedProcesses;

    int mTimeCounter; // Running time counter.
    int mTimeQuantum; // Time limit for each process.

  public:
    PriorityPreemptive(std::vector<Process>&);
    void scheduleProcess();
    void print();
    void printGanttChart();
};

PriorityPreemptive::PriorityPreemptive(std::vector<Process>& processList) {
    // Sort process based on arrival time.
    std::sort(processList.begin(), processList.end(), compare);

    // Load all processes to queue.
    for (Process& process : processList) {
        process.tempBurstTime = process.burstTime;
        mProcessList.push(process);
    }

    mTimeCounter = 0;
    mTimeQuantum = 1; // 1 second.
}

void PriorityPreemptive::scheduleProcess() {
    // Keep scheduling until all processes are processed.
    while (!mProcessList.empty() || !mRunningProcessList.empty()) {
        // If arrivalTime is less than mTimeCounter then load process into running Queue.
        while (!mProcessList.empty() && mTimeCounter >= mProcessList.front().arrivalTime) {
            mRunningProcessList.addProcess(mProcessList.front());
            mProcessList.pop();
        }

        if (!mRunningProcessList.empty()) {
            // Fetch least burst time process into currentProcess
            Process currentProcess = mRunningProcessList.getProcess();

            // Update starting time if required
            if (currentProcess.init == false) {
                currentProcess.startingTime = mTimeCounter;
                currentProcess.init = true;
            }
            mGantChart.push_back({currentProcess.pid, mTimeCounter}); // Push process to gant chart.

            if (currentProcess.tempBurstTime < mTimeQuantum) {
                mTimeCounter += currentProcess.tempBurstTime;
                currentProcess.tempBurstTime = 0;
            } else {
                currentProcess.tempBurstTime -= mTimeQuantum; // Process for time quantum.
                mTimeCounter += mTimeQuantum;
            }

            if (currentProcess.tempBurstTime == 0) {
                currentProcess.finishingTime = mTimeCounter; // Update finishing time.
                mFinishedProcesses.push_back(currentProcess);
            } else {
                // Add all the processes to the process list which
                // comes during the processing of the current process.
                for (int i = mTimeCounter - mTimeQuantum; i <= mTimeCounter; ++i) {
                    if (!mProcessList.empty() && i >= mProcessList.front().arrivalTime) {
                        mRunningProcessList.addProcess(mProcessList.front());
                        mProcessList.pop();
                    }
                }

                mRunningProcessList.addProcess(currentProcess);
            }
        } else {
            mTimeCounter += mTimeQuantum;
        }
    }

    // Update Finished processes table
    for (auto& p : mFinishedProcesses) {
        p.turnAroundTime = p.finishingTime - p.arrivalTime;
        p.waitingTime = p.turnAroundTime - p.burstTime;
    }
}

void PriorityPreemptive::print() {
    scheduleProcess();
    std::cout << "P\tAT\tPr\tBT\tST\tFT\tTAT\tWT\n";

    int waitingTime = 0;
    int turnAroundTime = 0;
    for (auto& p : mFinishedProcesses) {
        std::cout << p.pid << '\t';
        std::cout << p.arrivalTime << '\t';
        std::cout << p.priority << '\t';
        std::cout << p.burstTime << '\t';
        std::cout << p.startingTime << '\t';
        std::cout << p.finishingTime << '\t';
        std::cout << p.turnAroundTime << '\t';
        std::cout << p.waitingTime << '\n';

        waitingTime += p.waitingTime;
        turnAroundTime += p.turnAroundTime;
    }

    std::cout << std::endl;

    std::cout << "Average waiting time: " << double(waitingTime) / mFinishedProcesses.size() << std::endl;
    std::cout << "Average turn around time: " << double(turnAroundTime) / mFinishedProcesses.size() << std::endl;
    std::cout << std::endl;

    printGanttChart();
}

void PriorityPreemptive::printGanttChart() {
    std::cout << "Process: ";
    for (auto& p : mGantChart) {
        std::cout << p.first << '\t';
    }
    std::cout << std::endl;

    std::cout << "Time   : ";
    for (auto& p : mGantChart) {
        std::cout << p.second << '\t';
    }
    std::cout << std::endl;
}

int main() {
    // std::vector<Process> processList = {
    //    // pid, arrival time, priority, burst time
    //     {1, 0, 4, 4},
    //     {2, 0, 3, 3},
    //     {3, 1, 1, 1},
    //     {4, 2, 4, 4},
    //     {5, 1, 2, 2},
    //     {6, 4, 6, 6},
    // };

    std::vector<Process> processList = {
        // pid, arrival time, priority, burst time
        {1, 0, 1, 10},
        {2, 1, 2, 9},
        {0, 2, 1, 5},
        {4, 3, 2, 4},
        {5, 40, 5, 4},
    };

    PriorityPreemptive(processList).print();

    return 0;
}
