#include <cstdlib>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "OS.h"

// Helper function to remove the leading
//and trailing whitespace from a string
void OS::trim(std::string& s) 
{
    size_t p = s.find_first_not_of(" \t\r");
    s.erase(0, p);
    p = s.find_last_not_of(" \t\r");
    if (std::string::npos != p)
        s.erase(p+1);
}

bool OS::is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

bool OS::sort_by_start(Process* a, Process*b) {
    return (a->start < b->start || (a->start == b->start && a->id < b->id));
}

bool OS::sort_by_finish(Process* a, Process*b) {
    return (a->finish < b->finish || (a->finish == b->finish && a->id < b->id));
}

bool OS::sort_pair(const std::pair<int, int>& a, const std::pair<int, int>& b) {
    return (a.first < b.first);
}

bool OS::sort_by_position(Process* a, Process* b) {
    return a->positions[0].first < b->positions[0].first;
}

OS::OS(std::ifstream& infile, int memory_size) {
    std::string line;
    int n;
    // read in the arrive, burst time for each process
    while (std::getline(infile, line)) {
        trim(line);
        if (line[0] == '#') continue;
        if (is_number(line)) 
            n = atoi(line.c_str());
        else {
            std::stringstream ss;
            ss.str(line);
            std::string temp;
            char c;
            int m;
            std::vector<int> arr, run;
            int i = 0;
            while (ss >> temp) {
                if (i == 0) {
                    c = temp[0];
                } else if (i == 1) {
                    m = atoi(temp.c_str());
                } else {
                    size_t pos = temp.find('/');
                    arr.push_back(atoi(temp.substr(0, pos).c_str()));
                    run.push_back(atoi(temp.substr(pos+1).c_str()));
                }
                i++;
            }
            procs.push_back(Process(c, m, arr, run));
        }
    }
    if ( n != (int) procs.size() ) {
        std::cerr << "incorrect number of processes";
        exit( EXIT_FAILURE );
    }
    memory = std::string(memory_size, '.');
    // the clock used for simulation
    clock = 0;
    // initially, the whole memory is free momory
    free_memory.push_back(std::make_pair(0, memory_size-1));
    // cause nothing is used, we assume -1 is the last used memory.
    most_recent = -1;
    defrag = false;

    for (unsigned int i = 0; i < procs.size(); ++i) {
        waiting.push_back(&procs[i]);
    }
    waiting.sort(sort_by_start);// all processes are now in waiting queue
    free_memory.sort(sort_pair);// sort free memory positions by the positions

    infile.close();
}

// print the diagram of the memory
void OS::print_memory() {
    std::string temp(32, '=');
    std::cout << temp << std::endl;
    for (int i = 0; i < 8; ++i) {
        std::cout << memory.substr(memory.size()/8*i, memory.size()/8) << std::endl;
    }
    std::cout << temp << std::endl;
}

void OS::finish_process() {
    clock = (*running.begin())->finish;
    // finish the first element of running queue
    auto p = *running.begin();
    for (auto pos : p->positions) {
        for (int i = pos.first; i <= pos.second; ++i) {
            memory[i] = '.';
        }
        if (free_memory.empty()) {
            free_memory.push_back(pos);
        } else {
            auto q = free_memory.begin();
            // the newly released memory which was used used by finished process becomes free
            // find an appropriate position to insert into the free memory list
            while (q != free_memory.end()) {
                if (q->first > pos.first) {
                    q = free_memory.insert(q, pos);
                    break;
                }
                q++;
            }
            // if the newly released memory is after the last chunk of free memory
            if (q == free_memory.end()) {
                q = free_memory.insert(q, pos);
            }
            // merge if necessary
            // check if the newly released memory could be merged with the free memory right after itself
            if (q != free_memory.begin()) {
                auto prev = q;
                prev--;
                if (q->first == prev->second + 1) {
                    q->first = prev->first;
                    free_memory.erase(prev);
                }
            }
            // check if the newly released memory could be merged with the free memory right before itself
            auto post = q;
            post++;
            if (post != free_memory.end())
            {
                if (q->second + 1 == post->first) {
                    q->second = post->second;
                    free_memory.erase(post);
                }
            }
        }
    }
    if (--(p->burst_left) != 0) {
        // if the burst_left !=0, update the start finish time
        // and add into the waiting queue
        p->start = p->arr_time[p->arr_time.size() - p->burst_left];
        p->finish = p->start + p->run_time[p->arr_time.size() - p->burst_left];
        waiting.push_back(p);
        waiting.sort(sort_by_start);
    }
    std::cout << "time " << clock << "ms: Process " << p->id << " removed:" << std::endl;
    p->positions.clear();
    running.erase(running.begin());
}

void OS::start_process(int type) {
    clock = (*waiting.begin())->start;
    auto p = *waiting.begin();
    if (!defrag) {
    //if defragmentation does not happen right before this event,
    //we should output this message
        std::cout << "time " << clock << "ms: Process " << p->id << " arrived (requires " \
           << p->mem_size << " frames)" << std::endl;
    }
    int free_memory_size = 0; // the size of the available memory in total
    auto next_free_memory = free_memory.end();// the free memory right after most recently added process
                                              // it's the searching starting point for next-fit algorithm
    auto best_free_memory = free_memory.end();// the free memory that's gonna be filled for best fit algorithm
    int min = memory.size() + 1;
    auto worst_free_memory = free_memory.end();// the free memory that's gonna be filled for worst fit algorithm
    int max = 0;
    defrag = true; // suppose defrag could happen
    for (auto q = free_memory.begin(); q != free_memory.end(); ++q) {
        free_memory_size += (q->second - q->first + 1);
        if (next_free_memory == free_memory.end() && ( q->first >= most_recent + 1 || ( most_recent < q->second && \
            most_recent >= q->first ) ) ) {
            // only executed once, that's why we need the first condition
            // if q->first is after most_recent + 1, then it's the next free memory
            // the third condition is the corner case,
            // in which most_recent is within a free memory ( q->first <= most_recent < q->second)
            // this could happen when the memory before most_recent is freed before this start_process event 
            next_free_memory = q;
        }
        if (q->second - q->first + 1 >= p->mem_size) {
           defrag = false; // if any free memory is big enough, then defragmentation won't happen
           if (q->second - q->first + 1 < min) {
               best_free_memory = q;
               min = q->second - q->first + 1;
           } 
           if (q->second - q->first + 1 > max) {
               worst_free_memory = q;
               max = q->second - q->first + 1;
           }
        } 
    }
    if (next_free_memory == free_memory.end()) {
        next_free_memory = free_memory.begin(); //if the most_recent is within the last slot of the memory
                                                // next_free_memory should be the beginning.
    }
    if (free_memory_size < p->mem_size) {
        // if the free memory is not enough for the use of the newly added process, skip
        defrag = false;
        if (--(p->burst_left) != 0) {
            p->start = p->arr_time[p->arr_time.size() - p->burst_left];
            p->finish = p->start + p->run_time[p->arr_time.size() - p->burst_left];
            // re-sort waiting
            waiting.sort(sort_by_start);
        } else {
            waiting.erase(waiting.begin()); // if the number of bursts left is zero, erased from waiting queue
        }
        std::cout << "time " << clock << "ms: Cannot place process " << p->id << " -- skipped!" << std::endl;
        return;
    }
    // find which free memory to use (memory_to_use)
    // for contiguous algorithm, it has only one single value, 
    // that is the iterator of the free memory that's gonna be used
    // for noncontiguous algorithm, it will store the scattered free memory till enough spaces are accumulated.
    std::vector<std::list<std::pair<int, int> >::iterator> memory_to_use;
    switch (type) {
        case 0: {
            //for next fit algorithm, the searching starting point is the next_free_memory.
            auto q = next_free_memory;
            do {
                int start; 
                // if condition is the corner case mentioned before, 
                // in which, the most_recent is within a chunk of free memory
                if (most_recent < q->second && most_recent >= q->first\
                    && q->second - most_recent >= p->mem_size)    
                    // in this case, we should use the memory from most_recent + 1 instead of q->first!!!!
                    // And this is the only case when the q->first memory is not used
                    start = most_recent + 1; 
                else
                    //Otherwise, we start from q->first
                    start = q->first;
                if (q->second - start + 1 >= p->mem_size) {
                // when enough spaces are found, break the loop
                    break;
                }
                q++;
                // If searching till end of the free memory
                // and enough spaces are still not found,
                // go back from the start
                if (q == free_memory.end()) {
                    q = free_memory.begin();
                }
            } while (q != next_free_memory);
            memory_to_use.push_back(q);
            break;
        }
        case 1: { // best-fit
            memory_to_use.push_back(best_free_memory);
            break;
        }
        case 2: { // worst-fit
            memory_to_use.push_back(worst_free_memory);
            break;
        }
        case 3: { // non-contiguous
            // for non-contigous algorithm, the process must be able to be filled into the memory without defragmentation
            // if there are enough free spaces, so convert placed into true
            defrag = false;
            auto q = free_memory.begin();
            int count = 0;
            while (q != free_memory.end()) {
                memory_to_use.push_back(q);// store the free memory position information
                count += q->second - q->first + 1;
                if (count >= p->mem_size) {
                    //after enough spaces are accumulated
                    //break the loop
                    break;
                }
                q++;
            }
            break;
        }
    }
    if (!defrag) {
        int count = 0;
        for (auto q : memory_to_use) {
            int start;
            if (type == 0 && most_recent < q->second && most_recent >= q->first\
                && q->second - most_recent >= p->mem_size)
                start = most_recent + 1; 
            else 
                start = q->first;
            int i;
            for (i = start; i <= q-> second; ++i) {
            // fill the process in memory
            // when we get enough memory, break;
                memory[i] = p->id;
                count++;
                if (count == p->mem_size) {
                    break;
                }
            }
            if (i == q->second + 1) --i;
            //update the positions of memory which each process are using
            p->positions.push_back(std::pair<int, int>(std::make_pair(start, i)));

            if (type == 0 && most_recent < q->second && most_recent >= q->first\
                && q->second - most_recent >= p->mem_size) {
                int old_second = q->second;
                // in this corner case, the new q->second should be the old most_recent
                q->second = most_recent;
                most_recent = i;
                // check in the corner case, whether we should create a new chunk
                // of free memory between i + 1 and old_second
                if (most_recent + 1 <= old_second) {
                    free_memory.push_back(std::make_pair(most_recent + 1, old_second));
                    free_memory.sort(sort_pair);
                }
            }
            else {
                most_recent = i;
                q->first = most_recent + 1;
                if (q->first == q->second + 1) { // in this case the empty slot was used up
                    free_memory.erase(q);
                }
            }
            if (count == p->mem_size) {
                //break the big for loop in 315th line
                break;
            }
        }
        running.push_back(p);
        running.sort(sort_by_finish);
        std::cout << "time " << clock << "ms: Placed process " << (*waiting.begin())->id << ":" << std::endl;
        waiting.erase(waiting.begin());
    } else {
        // defragmentation
        std::cout << "time " << clock << "ms: Cannot place process " \
            << p->id << " -- starting defragmentation" << std::endl;
        defragmentation();
    }
}

void OS::schedule(int type) {
    std::string algorithm;
    switch (type) {
        case 0: {
            algorithm = "(Contiguous -- Next-Fit)";
            break;
        }
        case 1: {
            algorithm = "(Contiguous -- Best-Fit)";
            break;
        }
        case 2: {
            algorithm = "(Contiguous -- Worst-Fit)";
            break;
        }
        case 3: {
            algorithm = "(Non-contiguous)";
            break;
        }
    }
    std::cout << "time " << clock << "ms: Simulator started " << algorithm << std::endl;
    do {
        // find the next interesting event
        if (!running.empty() && !waiting.empty()) {
            if ((*running.begin())->finish <= (*waiting.begin())->start) {
                finish_process();
            } else {
                start_process(type);
            }
        } else if (running.empty() && !waiting.empty()) {
            start_process(type);
        } else if (!running.empty() && waiting.empty()) {
            finish_process();
        }
        print_memory();
    } while (!running.empty() || !waiting.empty());
    std::cout << "time " << clock << "ms: Simulator ended " << algorithm << std::endl;
}

void OS::defragmentation() {
    //find the first free memory(first_free_mem) to be used
    //in order to calculate how many frames have been moved
    //and find out which frames have been moved
    int first_free_mem = 0;
    for (unsigned int i = 0; i < memory.size(); ++i) {
        if (memory[i] == '.') {
            first_free_mem = i;
            break;
        }
    }
    auto running_copy = running;
    // sort the running queue by positions for defragmentation
    running_copy.sort(sort_by_position);
    
    //reassign the positions of the processes
    //and calculate how many frames have been moved.
    //all of the processes that are after the first_free_mem 
    //must have been moved
    int current_position = 0;
    int count_diff = 0;
    std::string moved_procs;
    for (auto ptr : running_copy) {
        int size = ptr->positions[0].second - ptr->positions[0].first + 1;
        if (ptr->positions[0].first > first_free_mem){ 
            count_diff += size;
            moved_procs.push_back(ptr->id);
        }
        for (int i = current_position; i <= current_position+size-1; ++i) {
            memory[i] = ptr->id;
        }
        ptr->positions[0].first = current_position;
        ptr->positions[0].second = current_position + size - 1;
        current_position += size;
    }
    // after defragmentation, the most_recent should be reset
    most_recent = - 1;
    // set the end of the memory to '.'
    for (unsigned int i = current_position; i < memory.size(); ++i) {
        memory[i] = '.';
    }

    //update the clock, start, finish, arrival time
    clock += count_diff;
    for (auto& proc : procs) {
        for (auto& t : proc.arr_time) t += count_diff;
        proc.start += count_diff;
        proc.finish += count_diff;
    }
    // reset the free_memory position
    free_memory.clear();
    free_memory.push_back(std::pair<int,int>(std::make_pair(current_position, memory.size()-1)));

    std::cout << "time " << clock << "ms: Defragmentation complete (moved " \
        << count_diff << " frames: ";
    unsigned int i;
    for (i = 0; i < moved_procs.size() - 1; ++i) {
        std::cout << moved_procs[i] << ", ";
    }
    std::cout << moved_procs[i] << ")" << std::endl;
}
