#include<queue>

using namespace std;

const int m(1);  // number of processor
const int t_cs(0); // context switch cost

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

typedef priority_queue<Process, vector<Process>, function<bool(const Process& proc1, const Process& proc2)>> pri_q;
