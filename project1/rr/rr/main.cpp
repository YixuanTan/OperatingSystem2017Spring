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
#include <iomanip>
#define DEBUG 0

using namespace std;

const int m(1);  // number of processor
const int t_cs(6); // context switch cost
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
    int turn_around;
    int wait_time;
    int preemption_count;
    int num_cs; // number of context switch
    int num_burst_left;
    int ini_turn_around;

    Process(string proc_id, int ini_arr_time, int burst_time, int num_bursts, int io_time) : proc_id(proc_id), ini_arr_time(ini_arr_time), burst_time(burst_time), num_bursts(num_bursts), io_time(io_time), arr_time(ini_arr_time), remaining_time(burst_time), wait_time(0), preemption_count(0), num_cs(0), num_burst_left(num_bursts), ini_turn_around(ini_arr_time) {}
    Process();
    
    
private:
    std::vector<std::string> split(std::string str); // helper function used by read
};


struct ready_greater {
    bool operator()(const Process& proc1, const Process& proc2){
        if(algorithm == "FCFS")
            return proc1.arr_time > proc2.arr_time || (proc1.arr_time == proc2.arr_time && proc1.proc_id > proc2.proc_id);
        else if(algorithm == "SRT")
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
            return proc1.arr_time < proc2.arr_time|| (proc1.arr_time == proc2.arr_time && proc1.proc_id < proc2.proc_id);
        else if(algorithm == "SRT")
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
protected:
    priority_queue<Process, vector<Process>, ready_greater> ready;
    priority_queue<Process, vector<Process>, blocked_greater> blocked;
    priority_queue<Process, vector<Process>, running_less> running;
    int num_cs; // number of context switches
    int num_pe; // number of preemptions
    int num_procs;
    int ts; // context switch time
    int sum_turn; // sum of turnaround time
    int sum_wait; // sum of waiting time
    int sum_burst_t; // sum of CPU bursting time
    int curr_time;
    int time_slice; // time slice
    stringstream output;
    string algorithm;
    int cs_until;
    int idleduration;
    int sum_num_bursts;
    
public:
    Scheduler(string, vector<Process>&);
    
    void gatherInfo(const Process&);
    string printReadyQueue(priority_queue<Process, vector<Process>, ready_greater>);
    void schedule(string);
    void printStatistics();
    
};

Scheduler::Scheduler(string algorithm, vector<Process>& processes){
    this->algorithm = algorithm;
    if(algorithm == "FCFS") {
        time_slice = INT_MAX;
    }
    else if (algorithm == "SRT") {
        time_slice = INT_MAX;
    }
    else if (algorithm == "RR") {
        time_slice = 94;
    }
    
    curr_time = 0;
    for(unsigned int i = 0; i < processes.size(); i++) {
        Process p = processes[i];
        blocked.push(p);
    }
    num_procs = (int)processes.size();
    sum_num_bursts = 0;
    idleduration = 0;
}

void Scheduler::gatherInfo(const Process& p) {
    num_cs += p.num_cs; // number of context switches
    num_pe += p.preemption_count; // number of preemptions
    //ts = t_cs * p.num_cs; // context switch time
    sum_turn += p.turn_around; // sum of turnaround time
    sum_wait += p.wait_time; // sum of waiting time
    sum_burst_t += p.num_bursts * p.burst_time; // sum of CPU bursting time
    sum_num_bursts += p.num_bursts;
}

void Scheduler::printStatistics(){
    double avg_burst_time = 1.0 * sum_burst_t / sum_num_bursts;
    double avg_wait_time = 1.0 * sum_wait / sum_num_bursts;
    double avg_turn_around = 1.0 * sum_turn / sum_num_bursts;
    cout << "Algorithm " << algorithm << endl;
    cout << "-- average CPU burst time: " << setprecision (2) << fixed << avg_burst_time << "ms" << endl;
    cout << "-- average wait time: " << setprecision (2) << fixed << avg_wait_time << " ms" << endl;
    cout << "-- average turnaround time: " << setprecision (2) << fixed << avg_turn_around << " ms" << endl;
    cout << "-- total number of context switches: " << num_cs << endl;
    cout << "-- total number of preemptions: " << num_pe << endl;
}

string Scheduler::printReadyQueue(priority_queue<Process, vector<Process>, ready_greater> pq) {
    
    stringstream ss;
    ss << "[Q ";
    if(pq.empty()) ss << "<empty>";
    while(!pq.empty()) {
        ss << pq.top().proc_id;
        pq.pop();
        if(!pq.empty()) ss << " ";
    }
    ss << "]";
    return ss.str();
}


class RR : public Scheduler{
public:
    void schedule(string);
    RR(string algorithm, vector<Process>& processes) : Scheduler(algorithm, processes) {}
};


void RR::schedule(string outputfile){
    cout << "time " << curr_time << "ms: Simulator started for " << algorithm << " " << printReadyQueue(ready) << endl;
    cs_until = 0;
    while(!ready.empty() || !blocked.empty() || !running.empty() || curr_time < cs_until - t_cs / 2) { // either ready/blocked queue is not empty
        if (DEBUG) cout << "curr time " << curr_time << endl;
        //check if jobs finished
        while(curr_time >= cs_until && !running.empty() && running.top().remaining_time == 0) {
            if(running.top().num_burst_left == 1) { // if it is the last burst
                Process runningtop = running.top();
                runningtop.turn_around += curr_time + t_cs / 2 - runningtop.ini_turn_around;
                gatherInfo(runningtop);
                cout << "time " << curr_time << "ms: Process " << runningtop.proc_id << " terminated " << printReadyQueue(ready) << endl;
            } else {
                Process runningtop = running.top();
                cout << "time " << curr_time << "ms: Process " << running.top().proc_id << " completed a CPU burst; " << --runningtop.num_burst_left << " burst" << (runningtop.num_burst_left == 1 ? "" : "s") << " to go " << printReadyQueue(ready) << endl;
                cout << "time " << curr_time << "ms: Process " << running.top().proc_id << " switching out of CPU; will block on I/O until time " << curr_time + t_cs / 2 + runningtop.io_time << "ms " << printReadyQueue(ready) << endl;
                
                runningtop.arr_time = curr_time + t_cs / 2 + runningtop.io_time;
                runningtop.remaining_time = runningtop.burst_time;
                runningtop.turn_around += curr_time + t_cs / 2 - runningtop.ini_turn_around;

                //runningtop.preemption_count++;
                //runningtop.num_cs++;
                blocked.push(runningtop);
            }
            running.pop();
            cs_until = curr_time + t_cs;
        }
        
        //check if time slice expired
        if(running.empty()) {
            if(idleduration == time_slice && ready.empty()) {
                //cout << "time " << curr_time << "ms: Time slice expired; no preemption because ready queue is empty!!! " << printReadyQueue(ready) << endl;
                idleduration = 1;
            }else {
                idleduration++;
            }
        }
        
        if (time_slice != INT_MAX && !running.empty() && curr_time - running.top().burst_start == time_slice) {
            Process runningtop = running.top();
            running.pop();
            if(ready.empty()) {
                //if(runningtop.num_burst_left > 0) {
                //runningtop.remaining_time = runningtop.burst_time;   remaining_time should NOT be reset when ready queue is empty
                runningtop.burst_start = curr_time;
                running.push(runningtop);
                ///}
                cout << "time " << curr_time << "ms: Time slice expired; no preemption because ready queue is empty " << printReadyQueue(ready) << endl;
            }else {
                cs_until = curr_time + t_cs;
                
                cout << "time " << curr_time << "ms: Time slice expired; process " << runningtop.proc_id << " preempted with " << runningtop.remaining_time << "ms to go " << printReadyQueue(ready) << endl;
                
                // update preemption
                runningtop.arr_time = curr_time + t_cs / 2 - 3;
                runningtop.preemption_count++;
                //runningtop.num_cs++;
                ready.push(runningtop);
            }
            idleduration = 1;
        }
        
        if (DEBUG) cout << "1111" << endl;
        while(curr_time >= cs_until && !ready.empty() && ready.top().arr_time + t_cs / 2 - 1 < curr_time) { // if ready queue front is good to run
            if(running.size() < m) {
                Process readytop = ready.top();
                readytop.burst_start = curr_time;
                readytop.wait_time += curr_time - readytop.arr_time - t_cs / 2;
                readytop.num_cs++;
                running.push(readytop);
                ready.pop();
                if(running.top().remaining_time == running.top().burst_time) {
                    cout << "time " << curr_time << "ms: Process " << running.top().proc_id << " started using the CPU " << printReadyQueue(ready) << endl;
                }
                else {
                    cout << "time " << curr_time << "ms: Process " << running.top().proc_id << " started using the CPU with " << running.top().remaining_time << "ms remaining " << printReadyQueue(ready) << endl;
                }
                
            } else {
                break;
            }
        }
        
        if (DEBUG) cout << "ready size:" << ready.size() << "  blocked size:" << blocked.size() << "  running size:" << running.size() << endl;
        while(!blocked.empty() && blocked.top().arr_time <= curr_time) {
            Process blockedtop = blocked.top();
            blockedtop.arr_time = curr_time; // arr_time means the time entering ready queue
            if(blockedtop.remaining_time == blockedtop.burst_time) blockedtop.ini_turn_around = curr_time;
            ready.push(blockedtop);
            if(blocked.top().arr_time == blocked.top().ini_arr_time)
                cout << "time " << curr_time << "ms: Process " << blocked.top().proc_id << " arrived and added to ready queue " << printReadyQueue(ready) << endl;
            else
                cout << "time " << curr_time << "ms: Process " << blocked.top().proc_id << " completed I/O; added to ready queue " << printReadyQueue(ready) << endl;
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
    cout << "time " << curr_time << "ms: Simulator ended for " << algorithm << endl;
    printStatistics();
    ofstream out;
    out.open(outputfile.c_str(), std::ofstream::out | std::ofstream::app);
    out << output.str();
    out.close();
}


int str2int(string s) {
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
        int ini_arr_time = str2int(token);
        if(!getline(ss, token, '|')) return false;
        int burst_time = str2int(token);
        if(!getline(ss, token, '|')) return false;
        int num_bursts = str2int(token);
        if(!getline(ss, token, '|')) return false;
        int io_time = str2int(token);
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
    RR rr(algorithm, input);
    rr.schedule(argv[2]);
    return 0;
}
