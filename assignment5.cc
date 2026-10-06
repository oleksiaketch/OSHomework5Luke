// Luke Oleksiak
// CS4420
// Fall 2026
// Assignment 5

#include <iostream> // used for cout
#include <iomanip> // used to format output text
#include <fstream> // used for file i/o
#include <vector> // vector class
#include <deque> // double sided queue
#include <algorithm> // algorithm library for sort function
#include <string> // string class
#include <cassert> // used for testing comparison functions
using namespace std; // standard namespace

struct Process // structure to hold a process
{
    int PID; // PID
    int arrTime; // arrival time
    int burTime; // burst time
    int timeLeft; // time left to complete the process (used in Round Robin algorithm)

    Process (int pid, int arr, int burst) // constructor that initializes the time left to the burst time
    {
        PID = pid; // assign pid
        arrTime = arr; // assign arrival
        burTime = burst; // assign burst time 
        timeLeft = burst; // assign time left to burst time
    }
};

struct FCResults // structure to hold a process that has finished, as well as the time it first started being processesed, the time it finished, and the waiting time
{
    Process proc;
    int startTime;
    int endTime;
    int waitingTime;
};

bool compareArrivals(const Process& processA, const Process& processB); // used to sort processes by arrival times
bool comparePID(const Process& processA, const Process& processB); // used to sort processes by PID
bool compareTimeRemaining(const Process& processA, const Process& processB); // used to sort processes by shortest time remaining
bool compareResultsPID(const FCResults& processA, const FCResults& processB); // used to sort finished processes by PID

void firstCome(deque<Process>& processes); // Uses first come first serve algorithm to simulate the processes
void roundRobin(deque<Process>& processes, int burstTime); // Uses round robin Algorithm to simulate the processes
void shortestFirst(deque<Process>& processes); // use shortest first algorithm to simulate the processes


int main(int argc, char* argv[])
{
    // command, input file, type of schedule to use, time quantum
    if(argc < 3 || argc > 4) // check if invalid number of arguments
    {
        cout << "Invalid number of arguments" << endl;
        return 1;
    }
    string algorithm = argv[2];
    if(algorithm == "RR" && argc != 4) // if round robin is chosen, check if a time quantum was entered
    {
        cout << "Error. Time quantum not found" << endl;
        return 1;
    }

    string inpfname = argv[1];
    ifstream infile(inpfname);
    if(!infile.is_open()) // check if input file can be opened
    {
        cout << "Could not open file" << endl;
        return 1;
    }

    int numberOfProcesses = 0;
    infile >> numberOfProcesses;
    if(numberOfProcesses <= 0) // check if number of processes > 0
    {
        cout << "Invalid number of processes" << endl;
        return 1;
    }
    deque<Process> readyQueue; // deque to store the processes that are read in from the file, is later passed to the algorithms to process
    int queueSize = 0;

    int currentPID, currentArrTime, currentBurTime;
    for(int i =0; i<numberOfProcesses; i++) // read in the information from the file
    {
        if(infile >> currentPID >> currentArrTime >> currentBurTime)
        {
            readyQueue.push_back({currentPID, currentArrTime, currentBurTime});
            queueSize++;
        }
        else
        {
            cout << "Error retrieving processes from file" << endl;
            return 1;
        }
    }
    if(queueSize != numberOfProcesses) // check if the expected number of processes were read from the file
    {
        cout << "Unexpected number of processes read" << endl;
        return 1;
    }

    string schedAlgo = argv[2]; // read the chosen scheduling algorithm from the command line
    if(schedAlgo != "FCFS" && schedAlgo != "SJF" && schedAlgo != "RR")
    {
        cout << "Scheduling algorithm not recognized." << endl;
        return 1;
    }
    cout << endl; // skip a line
    if(schedAlgo == "FCFS") // if first come is selected, run that algorithm
        firstCome(readyQueue);
    else  if (schedAlgo == "SJF") // if shortest job first is selected, run that algorithm
        shortestFirst(readyQueue);
    else // otherwise run the round robin algorithm if a valid time quantum is entered
    {
        string timeQuantST = argv[3]; // read in the time quantum
        int timeQuant = stoi(timeQuantST); // then convert it to an integer
        if(timeQuant <= 0) // if the time quantum entered is not a positive integer, error, terminate program
        {
            cout << "Invalid time quantum" << endl;
            return 1;
        }
        else // otherwise, run the Round Robin algorithm
            roundRobin(readyQueue, timeQuant);
    }
    // tests to verify correct logic execution for comparison functions
    assert(compareArrivals({0,0,5},{1,1,5}) == true); // Test that sooner arrival time for left process results in function returning true
    assert(compareArrivals({0,0,5},{1,0,5}) == true); // Test that same arrival time with lower PID for left process results in function returning true
    assert(compareArrivals({1,1,5},{0,0,5}) == false); // Test that sooner arrival time for right process results in function returning false
    assert(comparePID({0,0,5},{1,0,5}) == true); // Test that lower PID for left process results in function returning true
    assert(comparePID({1,0,5},{0,0,5}) == false); // Test that lower PID for the right process results in function returning false
    assert(compareTimeRemaining({0,0,4},{1,1,5}) == true); // Test that lower time remaining for left process returns true
    assert(compareTimeRemaining({0,0,8},{1,1,2}) == false); // Test that lower time remaining for right process returns false
    assert(compareTimeRemaining({0,0,5},{1,1,5}) == true); // Test that same time remaining for both processes but a lower PID for left process returns true
    cout << "Comparison function tests all passed as expected" << endl;
    return(0);
}// end of main

bool compareArrivals(const Process& processA, const Process& processB) // comparison function to sort processes by first arrival
{
    if(processA.arrTime != processB.arrTime) // if no tie in arrival times, use the first arrival time
        return processA.arrTime < processB.arrTime;
    else // use lower PID in case of a tie in arrival times
        return processA.PID < processB.PID;
}

bool comparePID(const Process& processA, const Process& processB) // comparison function to sort processes by PID
{
    return processA.PID < processB.PID;
}

bool compareResultsPID(const FCResults& processA, const FCResults& processB) // comparison function to sort processes by PID after processing them
{
    return processA.proc.PID < processB.proc.PID;
}

bool compareTimeRemaining(const Process& processA, const Process& processB) // comparison function to sort processes by lowest time remaining
{
    if(processA.burTime != processB.burTime) // if the processes have different burst times, return whether the first is lower
        return processA.burTime < processB.burTime;
    else // otherwise if same process time, return which arrived first
        return processA.arrTime < processB.arrTime;
}

void firstCome(deque<Process>& processes)
{
    //print header
    cout << " PID " << " Arrival Time " << " Start Time " << " End Time " << " Running Time " << " Waiting Time " << endl;
    // sort by arrival time
    sort(processes.begin(), processes.end(), compareArrivals);
    // deque to hold completed processes
    deque<FCResults> FCResults;
    int currentTime = 0; // the current time of the simulator
    int idleTime = 0; // total idle time (initialized to 0)
    for(int i=0; i<processes.size(); i++)
    {
        if(currentTime <processes[i].arrTime) // if the current time is before the arrival time of the next process, idle for the difference and go ahead to the time that process arrives
        {
            cout << right << setw(5) << "Idle" << setw(7) << "-" << setw(15) << currentTime << setw(11) << processes[i].arrTime << setw(12) << processes[i].arrTime - currentTime << endl;
            idleTime += processes[i].arrTime - currentTime;
            currentTime = processes[i].arrTime;
        }
        int endTime = currentTime+processes[i].burTime; // end time of current process = start time+burst time
        int waitingTime = currentTime - processes[i].arrTime; // calculate the amount of time this current process has waited to be executed
        cout << right << setw(3) << processes.at(i).PID << setw(9) << processes.at(i).arrTime << setw(15) << currentTime << setw(11) << endTime << setw(12) << processes.at(i).burTime << setw(13) << waitingTime << endl; // print out the details for this process
        processes.at(i).timeLeft = 0; // this process is complete, so set time left to 0
        FCResults.push_back({processes[i], currentTime, endTime,  waitingTime}); // add this completed process to the compled deque
        currentTime = endTime; // advance current time to the time the process completed
    }
    // calculate the total wait and average wait
    double totalWait =0;
    for(int i=0; i<FCResults.size(); i++)
    {
        totalWait += FCResults[i].waitingTime;
    }
    double averageWait = totalWait / FCResults.size();
    // skip a line, then print out the average waiting time
    cout << endl;
    cout << "Average waiting Time: " << averageWait << endl << endl;
}

void shortestFirst(deque<Process>& processes)
{
    // print header
    cout << " PID " << " Arrival Time " << " Start Time " << " End Time " << " Running Time " << " Waiting Time " << endl;
    //sort processes by arrival time
    sort(processes.begin(), processes.end(), compareArrivals); // sort processes by arrival times
    deque<Process> workQueue; // deque to hold the current queue of processes being worked on
    deque<FCResults> finished; // deque to hold finished processes
    // initialize the current time and the total idle time to 0
    int currentTime = 0;
    int idleTime = 0;

    while(!processes.empty() || !workQueue.empty()) // while there are current processes that have not finished running, keep running the simulation
    {
        while(!processes.empty() &&  processes.at(0).arrTime <= currentTime ) // add next processes to work queue if they have arrived, and remove them from the origin queue
        {
            workQueue.emplace_front(processes.at(0));
            processes.pop_front();
        }
        if(!workQueue.empty()) // if at least one process has arrived and is waiting to be done, do the one with the shortest remaining time
        {
            sort(workQueue.begin(), workQueue.end(), compareTimeRemaining); // re-sort work queue by shortest remaining time
            int endTime = currentTime + workQueue.at(0).burTime; // end time of next process is current time + the burst time of the process
            int waitingTime = (currentTime - workQueue.at(0).arrTime); // calculate the waiting time for this process
            workQueue.at(0).timeLeft = 0; // set the time left for this process to 0
            // print out the details for this process
            cout << right << setw(3) << workQueue.at(0).PID << setw(9) << workQueue.at(0).arrTime << setw(15) << currentTime << setw(11) << endTime << setw(12) << workQueue.at(0).burTime << setw(13) << waitingTime << endl; 
            finished.push_back({workQueue.at(0), currentTime, endTime, waitingTime }); // add this process to the completed processes deque
            workQueue.pop_front(); // remove this process from the work queue
            currentTime = endTime; // advance time to the end time of this process
        }
        else
        {
            //idle time until next process arrives. print out
            idleTime += processes.front().arrTime - currentTime;
            cout << right << setw(5) << "Idle" << setw(7) << "-" << setw(15) << currentTime << setw(11) << processes.front().arrTime << setw(12) << processes.front().arrTime - currentTime << endl;
            currentTime = processes.front().arrTime; // advance time to the time the next process arrives
        }
    }
    //calculate the total and average wait
    double totalWait =0;
    for(int i=0; i<finished.size(); i++)
    {
        totalWait += finished[i].waitingTime;
    }
    double averageWait = totalWait / finished.size();
    //print the average wait
    cout << endl;
    cout << "Average Waiting Time: " << averageWait << endl << endl;
}

void roundRobin(deque<Process>& processes, int burstTime)
{
    // print header
    cout << " PID " << " Start Time " << " End Time " << " Running Time " << endl;
    //sort processes by arrival time
    sort(processes.begin(), processes.end(), compareArrivals);
    //deque to hold the current work queue of processes that have arrived
    deque<Process> workQueue;
    //deque to hold the finished processes
    deque<FCResults> finished;
    // initialize current time and total idle time to 0
    int currentTime = 0;
    int idleTime = 0;
    while(!processes.empty() || !workQueue.empty())  // while there are still processes to work on
    {
        for(int i = processes.size()-1; i >=0; i--) // for all processes that have not been added to work queue yet, 
        {
            if(processes.at(i).arrTime <= currentTime) // if they have arrived by the current time, add them to the work queue and remove them from the original waiting queue
            {
                workQueue.emplace_front(processes.at(i));
                processes.erase(processes.begin()+i);
            }
        }

        if(!workQueue.empty()) // if at least one process has arrived and is waiting to be done, do the one at the front of the queue
        {
            //if time remaining on next process is >= TQ
            if(workQueue.front().timeLeft >= burstTime)
            {
                // advance time by the burst time
                int endTime = currentTime + burstTime;
                // print out run info for this burst
                cout << right << setw(3) << workQueue.at(0).PID << setw(9) << currentTime << setw(11) << endTime << setw(12) << burstTime << endl;                
                workQueue.front().timeLeft -= burstTime; // remove the burst time from the time left to finish this process
                if(workQueue.front().timeLeft == 0) // if the current task is now done, remove it from the work queue and add it to the finished queue
                {
                    int waitingTime = (endTime - workQueue.at(0).arrTime)-workQueue.front().burTime;
                    finished.push_back({workQueue.at(0), currentTime, endTime, waitingTime });
                    workQueue.pop_front();
                }
                else // otherwise task has more processing left after current burst, move the element to the back of the working queue
                {
                    workQueue.push_back({workQueue.front()});
                    workQueue.pop_front();
                }
                currentTime = endTime; // advance time to the end of the burst
            }
            else // task has less time left than the burst time
            {
                // end time is current time plus the time left to finish the process
                int endTime = currentTime + workQueue.front().timeLeft;
                // print out info for this burst time
                cout << right << setw(3) << workQueue.at(0).PID << setw(9) << currentTime << setw(11) << endTime << setw(12) << endTime-currentTime << endl;                
                int waitingTime = (endTime - workQueue.at(0).arrTime)-workQueue.front().burTime; // calculate total waiting time for the process
                workQueue.front().timeLeft = 0;
                // add process to the finished queue
                finished.push_back({workQueue.at(0), currentTime, endTime, waitingTime });
                workQueue.pop_front(); // remove process from the work queue
                //advance time to the end of the burst
                currentTime = endTime;
            }
        }
        else
        {
            // no process in work que. idle time, print out
            idleTime += processes.front().arrTime - currentTime;
            cout << right << setw(5) << "Idle" << setw(7) << currentTime << setw(11) << processes.front().arrTime << setw(12) << processes.front().arrTime - currentTime << endl;
            currentTime = processes.front().arrTime; // advance time to the time the next process arrives
        }
    }
    sort(finished.begin(), finished.end(), compareResultsPID); // sort results by pid
    cout << endl << " PID " << " Arrival Time " << " Running Time " << " End Time "  << " Waiting Time " << endl; // print out headewr
    // print out info for the finished processes
    for(int i = 0; i < finished.size(); i++)
        cout << right << setw(3) << finished.at(i).proc.PID << setw(9) << finished.at(i).proc.arrTime << setw(15) << finished.at(i).proc.burTime << setw(11) << finished.at(i).endTime << setw(13) << finished.at(i).waitingTime << endl; 
    // calculate total and average wait
    double totalWait =0;
    for(int i=0; i<finished.size(); i++)
    {
        totalWait += finished[i].waitingTime;
    }
    double averageWait = totalWait / finished.size();
    // print average waiting time
    cout << endl;
    cout << "Average Waiting Time: " << averageWait << endl << endl;
} // end of function