#include <string>
#include <vector>
#include <list>
#include <set>
#include <fstream>

class Process {
public:
    std::string proc_id;
    int ini_arr_t; //initial arrive time
    int burst_t; // burst_time
    int num_bursts; // number of bursts
    int io_t; // IO running time
    
    // The following variables are used by OS
    int curr_arr_t; //current_arrive_time
    int burst_left;
    int num_left;
    int turn_start;
    std::vector <int> turnaround;
    std::vector <int> waittime;
    bool read(std::ifstream & in_str);
private:
	std::vector<std::string> split(std::string str); // helper function used by read
};

// This class handles the processes.
class OS {
private:
    std::list<Process*> READY; // queue of ready jobs
    std::set<Process*> RUNNING; // running jobs
    std::list<Process*> BLOCKED; // queue of io jobs
    std::vector<Process> procs; // all the jobs to be done
    int num_cs; // number of context switches
    int num_pe; // number of preemptions
    int nprocs; // number of processors
    int ts; // context switch time
    int sum_turn; // sum of turnaround time
    int sum_wait; // sum of waiting time
    int sum_burst_t; // sum of CPU bursting time
    int time_slice; // time slice
public:
    static bool FCFS_sort(Process* p, Process* q); // comparison function for FCFS
    static bool SJF_sort(Process* p, Process* q); // comparison function for SJF
    OS(const std::vector<Process>& processes, const int m, const int time_switch, const int time_slice_ ) : \
    procs(processes), nprocs(m), ts(time_switch), time_slice(time_slice_) {}; // constructor for RR, with an argument for time slice

    OS(const std::vector<Process>& processes, const int m, const int time_switch );// constructor for FCFS and SJF, without time slice
    void print_READY(int t, const std::string& message); // function to print READY queue with a message
    void update_READY(int t, bool (*sort_procs)(Process* p, Process* q)); // update the READY queue at time t, with a corresponding comparison function for each algorithm
    void schedule(bool (*sort_procs)(Process* p, Process* q), const std::string& algo); // function to schedule the processes with a specific scheduling algorithm
    void check_io(int, Process*, bool (*sort_procs)(Process* p, Process* q)); // check if there is a process finish I/O
    void report_result(const std::string&, const std::string& ); // report the statistics
    void reset(); // reset the OS
};

