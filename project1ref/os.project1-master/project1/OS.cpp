#include "OS.h"
#include <cstdlib>
#include <algorithm>
#include <ios>
#include <iostream>

std::vector<std::string> Process::split(std::string str) {
    std::vector<std::string> result;
    std::string pattern = "|";
    str+=pattern;
    int size=str.size();
    for(int i=0; i<size; i++){
        int pos=str.find(pattern,i);
        if(pos<size){
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i=pos+pattern.size()-1;
        }
    }
    return result;
}

bool Process::read(std::ifstream& in_str) {
	std::string line;
	while (std::getline(in_str, line)) {
		if (line[0] == '#') continue;
		std::vector<std::string> a = split(line);
        proc_id = a[0];
       	ini_arr_t = std::atoi(a[1].c_str());
       	burst_t = std::atoi(a[2].c_str());
       	num_bursts = std::atoi(a[3].c_str());
       	io_t = std::atoi(a[4].c_str());

        curr_arr_t = ini_arr_t;
        num_left = num_bursts;
        burst_left = burst_t;
       	return true;
	}
	return false;
}

OS::OS(const std::vector<Process>& processes, const int m, const int time_switch)
{
	procs = processes;
	nprocs  = m;
	ts = time_switch;
	time_slice = 0;
	for (unsigned int i = 0; i< procs.size(); ++i) {
	    time_slice = procs[i].burst_t > time_slice ? procs[i].burst_t : time_slice;
	}
};

bool OS::FCFS_sort(Process* p, Process* q) {
	return (p->curr_arr_t < q->curr_arr_t || (p->curr_arr_t == q->curr_arr_t && p->proc_id < q->proc_id));
}

bool OS::SJF_sort(Process* p, Process* q) {
	return (p-> burst_t < q-> burst_t || (p->burst_t == q->burst_t && p->proc_id < q->proc_id));
}

//Update ready queue content with current time
void OS::update_READY(int t, bool (*sort_procs)(Process*, Process*)) {
    std::vector<Process*> ready_processes; // newly inserted processes
    for (unsigned int i = 0; i < procs.size(); ++i) {
        // push to READY queue if it arrives before t and it is not terminated and it is not in READY or RUNNING queue already
        if (procs[i].curr_arr_t <= t && procs[i].num_left > 0 \
            && std::find(READY.begin(), READY.end(), &procs[i]) == READY.end() && \
            RUNNING.find(&procs[i]) == RUNNING.end()) {
            ready_processes.push_back(&procs[i]);
        }
    }

    std::sort(ready_processes.begin(), ready_processes.end(), FCFS_sort); // sort the READY queue chronologically
    for (unsigned int i = 0; i<ready_processes.size(); ++i) { 
        READY.push_back(ready_processes[i]);
    	// sort READY Queue according to the selected algorithm
        READY.sort(sort_procs);
    	// print a message if this process is added into READY queue for the first time
    	if (ready_processes[i]->num_left == ready_processes[i]->num_bursts && \
	        ready_processes[i]->burst_t == ready_processes[i]->burst_left)
            print_READY(ready_processes[i]->curr_arr_t, "Process " + ready_processes[i]->proc_id + " arrived");
    }
}

void OS::print_READY(int t, const std::string& message) { // print the READY queue in the following format: time, message, content of queue
    std::cout << "time " << t << "ms: " << message << " [Q";
    if (!READY.empty()) {
    	for (std::list<Process*>::const_iterator itr = READY.begin(); itr != READY.end(); ++itr) {
            std::cout << " " << (*itr)->proc_id;
    	}
    } else {
	std::cout << " empty";
    }
    std::cout<< "]" <<std::endl;
}

// check if there is any process in the IO queue finishing before a certain time
void OS::check_io(int t_, Process* current_, bool (*sort_procs_)(Process* p, Process* q)) {
    std::list<Process*>::iterator itr = BLOCKED.begin();
    while (itr != BLOCKED.end()) {
        if (t_ >= (*itr)->curr_arr_t) {
            update_READY((*itr)->curr_arr_t, sort_procs_);
            print_READY((*itr)->curr_arr_t, "Process " + (*itr)->proc_id + " completed I/O");
            itr = BLOCKED.erase(itr);
        } else {
            itr++;
        }
    }
}

void OS::schedule(bool (*sort_procs_)(Process*, Process*), const std::string& algo) {
    int t = 0; // CPU time
    num_cs = 0;
    num_pe = 0;
    std::cout << "time 0ms: Simulator started for " << algo <<" [Q empty]" << std::endl;
    update_READY(t, sort_procs_);
    // schedule jobs till all processes are terminated
    do {
        // add jobs with arrival times <= current time to ready queue
        update_READY(t,sort_procs_);
        if (!READY.empty()) {
            // if there is a job waiting in READY queue
            Process* current = *READY.begin();
            READY.erase(READY.begin());
            if (current->burst_left == current -> burst_t) {
                current->turn_start = current->curr_arr_t; // record the arrival time as the start of turnarount/wait time
            }
            current->waittime.push_back(t - current->curr_arr_t); // calculate the wait time for this burst
            t += ts/2; // switch in
            ++num_cs;
            RUNNING.insert(current); // add the current process into the RUNNING queue
            print_READY(t, "Process " + current->proc_id + " started using the CPU");

            bool preempt = current->burst_left > time_slice; // An indicator of the preemption
            while (preempt){
                // if preemption occurs, report it
                check_io(t + time_slice, current, sort_procs_);
                t += time_slice;
                current->burst_left -= time_slice;
                update_READY(t, sort_procs_);
                if (!READY.empty()){
                    // if the READY queue is not empty when preemption occurs, move the current process into the READY queue
                    //  and continue scheduling the next process (GOTO current->curr_arr_t = t + ts/2)
                    READY.push_back(current);
                    ++num_pe;
                    print_READY(t,"Time slice expired; process " + current->proc_id + \
                        " preempted with " + std::to_string(current->burst_left) + "ms to go" );
                    break;
                } else {
                    // if the READY queue is empty when preemption occurs, do not kick the current process out
                    // after another time slice, check if it is going to be preempted again
                    print_READY(t, "Time slice expired; no preemption because ready queue is empty");
                    preempt = current->burst_left > time_slice;
                } 
            }

            if (!preempt) {
                // if the process is not preempted, it should finish the current CPU burst
                check_io(t + current->burst_left, current, sort_procs_);
                t += current->burst_left;
                update_READY(t, sort_procs_);
                (current->num_left)--;
                current->turnaround.push_back(t - current->turn_start);
                if (current->num_left > 0) {
                    print_READY(t, "Process " + current->proc_id + " completed a CPU burst; " + \
                        std::to_string(current->num_left) + " to go");
                    current->burst_left = current->burst_t;
                } else {
                    print_READY(t, "Process " + current->proc_id + " terminated");
                }
            }

            current->curr_arr_t = t + ts/2; // set the arrival time for the next burst
            update_READY(t, sort_procs_); // After a running job finishes, the READY queue should be updated
            t += ts/2; // switch out
            update_READY(t, sort_procs_); // update again as the time ellapses
            RUNNING.erase(current);

            // Process enters BLOCKED if it needs to
            if (current->num_left > 0 && current->io_t > 0 && !preempt) {
                print_READY(t - ts/2, "Process " + current->proc_id + " blocked on I/O until time " + std::to_string \
                (t - ts/2 + current->io_t) + "ms");
                BLOCKED.push_back(current);
                current->curr_arr_t = (t + current->io_t - ts/2);
            } 
        } else if (!BLOCKED.empty()) {
            // if there is no job waiting to be processed, check the BLOCKED queue
            BLOCKED.sort(FCFS_sort);
            update_READY((*BLOCKED.begin())->curr_arr_t, FCFS_sort);
            print_READY((*BLOCKED.begin())->curr_arr_t, "Process " + (*BLOCKED.begin())->proc_id + " completed I/O");
            t = (*BLOCKED.begin())->curr_arr_t;
            BLOCKED.erase(BLOCKED.begin()); // only erase the first one, then continue scheduling
        }
    } while (!READY.empty() || !BLOCKED.empty());
    std::cout << "time " << t << "ms: Simulator ended for " << algo << std::endl;
}

void OS::report_result(const std::string& filename, const std::string& algo) {
    sum_burst_t = 0;
    sum_turn = 0;
    sum_wait = 0;
    int sum_burst = 0; 
    for (unsigned int i = 0; i < procs.size(); ++i) {
        sum_burst += procs[i].num_bursts;
        for (unsigned int j = 0; j < procs[i].turnaround.size(); ++j) {
            sum_turn += procs[i].turnaround[j];
        }
        for (unsigned int j = 0; j < procs[i].waittime.size(); ++j) {
            sum_wait += procs[i].waittime[j];
        }
        sum_burst_t += procs[i].num_bursts * procs[i].burst_t;
    }
    std::ofstream out_str(filename, std::ofstream::out | std::ofstream::app);
    out_str.precision(2);
    out_str << "Algorithm " << algo << std::endl;
    out_str << std::fixed;
    out_str << "-- average CPU burst time: "<< sum_burst_t * 1.0 / sum_burst << " ms" << std::endl;
    out_str << "-- average wait time: " << sum_wait * 1.0 / sum_burst << " ms" << std::endl;
    out_str << "-- average turnaround time: " <<sum_turn * 1.0 / sum_burst << " ms" << std::endl;
    out_str << "-- total number of context switches: " << num_cs << std::endl;
    out_str << "-- total number of preemptions: " << num_pe << std::endl;
    out_str.close();
}

void OS::reset() {
    for (unsigned int i = 0; i < procs.size(); ++i) {
        procs[i].curr_arr_t = procs[i].ini_arr_t;
        procs[i].num_left = procs[i].num_bursts;
        procs[i].turnaround.clear();
        procs[i].waittime.clear();
        procs[i].burst_left = procs[i].burst_t;
    }
}
