#include <vector>
#include <utility>

class Process {
public:
    char id;
    int mem_size;
    std::vector<int> arr_time;
    std::vector<int> run_time;
    int start;//process starts time
    int finish; //process finishes time
    int burst_left;//number of process bursts left
    //the positions of chunks of memories process uses. 
    // for non-contiguous, there are multiple pairs
    // for contiguous, there is only one pair
    std::vector<std::pair<int, int> > positions; 
    Process(char c, int m, const std::vector<int>& arr, const std::vector<int>& run) : \
        id(c), mem_size(m), arr_time(arr), run_time(run), \
        start(arr[0]), finish(arr[0]+run[0]), burst_left(arr.size()) {};
};
