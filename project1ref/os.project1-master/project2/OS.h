#include <string>
#include <vector>
#include <utility>
#include <list>
#include <fstream>
#include "process.h"

class OS {
private:
    void trim(std::string& s);
    bool is_number(const std::string& s);
    bool defrag; //whether defragmentation could happen
    void defragmentation(); // reorganize memory, update running, free_memory, waiting
    static bool sort_by_start(Process* a, Process* b);
    static bool sort_by_finish(Process* a, Process* b);
    static bool sort_pair(const std::pair<int, int>& a, const std::pair<int, int>& b);
    static bool sort_by_position(Process* a, Process* b);
    void start_process(int type);
    void finish_process();
private:
    std::vector<Process> procs;
    std::string memory;
    int clock;
    std::list<std::pair<int, int> > free_memory; // positions of chunks of free memory 
    // any process is in either running queue or waiting queue
    std::list<Process*> running; // processes in running status, sort_by_finish
    std::list<Process*> waiting; // processes in waiting status, sort_by_start
    // most_recent indicates the end of the most recently placed process
    int most_recent;
public:
    OS(std::ifstream& infile, int memory_size);
    void schedule(int type);
    void print_memory();
};
