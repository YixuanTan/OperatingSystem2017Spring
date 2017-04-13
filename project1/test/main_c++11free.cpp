//
//  main.cpp
//  project1
//
//  Created by Yixuan Tan on 2/23/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//

#include <iostream>
#include <queue>
#include <vector>
#include <sstream>
#include <fstream>
#include <queue>
//#include <functional>
#include <limits.h>

using namespace std;

const int m(1);  // number of processor
const int t_cs(0); // context switch cost
string algorithm;

class Process {
public:
    // input variables
    std::string proc_id;
    int ini_arr_time;
    int burst_time;
    int num_bursts;
    int io_time;
    
    // The following variables are used by OS
    int arr_time; // arr_time for the most recent "arrive" if in ready queue, or the next arrive time if in blocked queue
    int burst_start;
    int remaining_time;
    int wait_time;
    int preemption_count;
    int num_cs; // number of context switch

    Process(string proc_id, int ini_arr_time, int burst_time, int num_bursts, int io_time) : proc_id(proc_id), ini_arr_time(ini_arr_time), burst_time(burst_time), num_bursts(num_bursts), io_time(io_time), arr_time(ini_arr_time), remaining_time(burst_time), wait_time(0), preemption_count(0), num_cs(0){}
    Process();


private:
    std::vector<std::string> split(std::string str); // helper function used by read
};


#define DEBUG 0
struct ready_greater {
    bool operator()(const Process& proc1, const Process& proc2){
        if(algorithm == "FCFS")
            return proc1.ini_arr_time > proc2.ini_arr_time || (proc1.ini_arr_time == proc2.ini_arr_time && proc1.proc_id > proc2.proc_id);
        else if(algorithm == "SRF")
            return proc1.remaining_time > proc2.remaining_time || (proc1.remaining_time == proc2.remaining_time && proc1.proc_id > proc2.proc_id);
        else if(algorithm == "RR")
            return proc1.arr_time > proc2.arr_time|| (proc1.arr_time == proc2.arr_time && proc1.proc_id > proc2.proc_id);
        else {
            cerr << "algorithm is wrong %s" << algorithm << endl;
            return 1;
        }
    }
};

struct running_less{
    bool operator()(const Process& proc1, const Process& proc2){
        if(algorithm == "FCFS")
            return proc1.ini_arr_time < proc2.ini_arr_time|| (proc1.ini_arr_time == proc2.ini_arr_time && proc1.proc_id < proc2.proc_id);
        else if(algorithm == "SRF")
            return proc1.remaining_time < proc2.remaining_time || (proc1.remaining_time == proc2.remaining_time && proc1.proc_id < proc2.proc_id);
        else if(algorithm == "RR")
            return proc1.burst_start > proc2.burst_start || (proc1.burst_start == proc2.burst_start && proc1.proc_id > proc2.proc_id);
        else {
            cerr << "algorithm is wrong" << endl;
            return 1;
        }

    }
};

struct blocked_greater{
    bool operator()(const Process& proc1, const Process& proc2){
        return proc1.arr_time > proc2.arr_time || (proc1.arr_time == proc2.arr_time && proc1.proc_id > proc2.proc_id);
    }
};




class Scheduler {
private:
    priority_queue<Process, vector<Process>, ready_greater> ready;
    priority_queue<Process, vector<Process>, blocked_greater> blocked;
    priority_queue<Process, vector<Process>, running_less> running;
    int num_cs; // number of context switches
    int num_pe; // number of preemptions
    int nprocs; // number of processors
    int ts; // context switch time
    int sum_turn; // sum of turnaround time
    int sum_wait; // sum of waiting time
    int sum_burst_t; // sum of CPU bursting time
    int curr_time;
    int time_slice; // time slice
    stringstream output;
    string algorithm;

public:
    Scheduler(string, vector<Process>&);
    
    void gatherInfo(const Process&);
    string printReadyQueue(priority_queue<Process, vector<Process>, ready_greater>);
    bool time_slice_check(int);
    void schedule(string);

};


Scheduler::Scheduler(string algorithm, vector<Process>& processes){
    if(algorithm == "FCFS") {
        time_slice = INT_MAX;
    }
    else if (algorithm == "SRF") {
        time_slice = INT_MAX;
    }
    else if (algorithm == "RR") {
        time_slice = 3;
    }
    
    curr_time = 0;
    for(unsigned int i = 0; i < processes.size(); i++) {
        Process p = processes[i];
        blocked.push(p);
    }
}


void Scheduler::schedule(string outputfile){
    output << "Start of simulation" << endl;

    while(!ready.empty() || !blocked.empty() || !running.empty()) { // either ready/blocked queue is not empty
        if (DEBUG) cout << "curr time " << curr_time << endl;
        //check if jobs finished
        while(!running.empty() && running.top().remaining_time == 0) {
            gatherInfo(running.top());
            output << "time <" << curr_time << ">ms: Process " << running.top().proc_id << " terminates by finishing its last CPU burst " << printReadyQueue(ready) << endl;
            running.pop();
        }
        if (DEBUG) cout << "1111" << endl;
        while(!ready.empty() && ready.top().arr_time < curr_time) { // if ready queue front is good to run
            if(running.size() < m) {
                Process readytop = ready.top();
                readytop.burst_start = curr_time;
                readytop.wait_time += curr_time - 1 - readytop.arr_time;
                readytop.num_cs++;
                running.push(readytop);
                ready.pop();
                output << "time <" << curr_time << ">ms: Process " << running.top().proc_id << " starts using the CPU " << printReadyQueue(ready) << endl;
            }else if(running.size() == m && (running_less()(ready.top(), running.top()) || (time_slice != INT_MAX && time_slice_check(curr_time)))) {
                Process runningtop = running.top();
                running.pop();
                output << "time <" << curr_time << ">ms: Process " << runningtop.proc_id << " is preempted" << printReadyQueue(ready) << endl;

                // update preemption
                runningtop.arr_time = curr_time + t_cs / 2 + runningtop.io_time - 1;
                runningtop.preemption_count++;
                runningtop.num_cs++;
                blocked.push(runningtop);
                output << "time <" << curr_time << ">ms: Process " << runningtop.proc_id << " starts performing I/O" << printReadyQueue(ready) << endl;

                Process readytop = ready.top();
                ready.pop();
                // update wait time
                readytop.burst_start = curr_time + t_cs / 2;
                readytop.wait_time += curr_time - 1 - readytop.arr_time;
                readytop.num_cs++;
                running.push(readytop);
                output << "time <" << curr_time << ">ms: Process " << running.top().proc_id << " starts using the CPU " << printReadyQueue(ready) << endl;

            } else {
                break;
            }
        }
        if (DEBUG) cout << "ready size:" << ready.size() << "  blocked size:" << blocked.size() << "  running size:" << running.size() << endl;
        while(!blocked.empty() && blocked.top().arr_time <= curr_time) {
            if(blocked.top().arr_time == blocked.top().ini_arr_time){
                output << "time <" << curr_time << ">ms: Process " << blocked.top().proc_id << " arrives " << printReadyQueue(ready) << endl;
            }
            ready.push(blocked.top());
            blocked.pop();
            
        }

        // update remaining time
        
        vector<Process> runningstore;
        while(!running.empty()) {
            runningstore.push_back(running.top());
            running.pop();
            runningstore.back().remaining_time--;
        }
        for(unsigned int i = 0; i < runningstore.size(); i++) {
            Process p = runningstore[i];
            running.push(p);
        }
        curr_time++;
    }
    output << "End of simulation" << endl;

    ofstream out;
    out.open(outputfile.c_str(), std::ofstream::out | std::ofstream::app);
    out << output.str();
    out.close();
}


void Scheduler::gatherInfo(const Process& p) {
    num_cs += p.num_cs; // number of context switches
    num_pe += p.preemption_count; // number of preemptions
    nprocs++; // number of processors
    ts = t_cs * p.num_cs; // context switch time
    sum_turn += curr_time - 1 - p.ini_arr_time; // sum of turnaround time
    sum_wait += p.wait_time; // sum of waiting time
    sum_burst_t += p.burst_time; // sum of CPU bursting time
}

string Scheduler::printReadyQueue(priority_queue<Process, vector<Process>, ready_greater> pq) {
    stringstream ss;
    ss << "[Q <";
    while(!pq.empty()) {
        ss << pq.top().proc_id << " ";
        pq.pop();
    }
    ss << ">]";
    return ss.str();
}

bool Scheduler::time_slice_check(int curr_time){
    if(curr_time - running.top().burst_start == time_slice) {
        if(!ready.empty()) return true;
        else {
            Process runningtop = running.top();
            running.pop();
            runningtop.burst_start = curr_time;
        }
    }
    return false;
}

int stoi(string s) {
    int num = 0;
    for(unsigned int i = 0; i < s.length(); i++) {
        num = num * 10 + s[i] - '0';
    }
    return num;
}

bool readInput(vector<Process>& input, string inputfile){
    /*
    input.push_back({"A", 0, 168, 5, 287});
    input.push_back({"B", 0, 385, 1, 0});
    input.push_back({"C", 190, 97, 5, 2499});
    input.push_back({"D", 250, 1770, 2, 822});
    */
    /*
    input.push_back({"A", 0, 12, 1, 0});
    input.push_back({"B", 1, 9, 1, 0});
    input.push_back({"C", 4, 4, 1, 0});
    input.push_back({"D", 7, 3, 1, 0});
     */
    ifstream read(inputfile.c_str());
    if (!read.good()) {
        std::cerr << "Can not open " << inputfile << " to read." << std::endl;
        return 1;
    }

    string line, token;
    while(read.peek() == '#') {
        getline(read, line);
    }
    
    while(getline(read, line)) {
        stringstream ss(line);
        if(!getline(ss, token, '|')) return false;
        string proc_id = token;
        if(!getline(ss, token, '|')) return false;
        int ini_arr_time = stoi(token);
        if(!getline(ss, token, '|')) return false;
        int burst_time = stoi(token);
        if(!getline(ss, token, '|')) return false;
        int num_bursts = stoi(token);
        if(!getline(ss, token, '|')) return false;
        int io_time = stoi(token);
        input.push_back(Process(proc_id, ini_arr_time, burst_time, num_bursts, io_time));
    }
    
    read.close();
    return true;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    if (argc != 3) {
        cerr << "ERROR: Invalid arguments \nUsage: " << argv[0] << "<input-file> <stats-output-file>" << endl;
        return 1;
    }
    
    vector<Process> input;
    if(!readInput(input, argv[1])){
        cerr << "ERROR: Invalid input file format" << endl;
        return 1;
    }
    algorithm = "RR";
    Scheduler os(algorithm, input);
    os.schedule(argv[2]);
    return 0;
}
