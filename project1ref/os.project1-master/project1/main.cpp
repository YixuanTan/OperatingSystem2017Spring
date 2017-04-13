/*
 * Operating Systems Project 1
 * Team members: Jie Cheng, Wei Peng, Ni Zhang
 * */

#include <iostream>
#include <fstream>
#include "OS.h"

int main(int argc, const char * argv[]) {
    if (argc != 3) {
        std::cerr <<"Usage: " << argv[0] << "<input file> <stats-output-file>\n";
        return 1;
    }
    
    std::ifstream in_str(argv[1]);
    if (!in_str.good()) {
        std::cerr << "Can not open " <<argv[1] << " to read." << std::endl;
        return 1;
    }

    std::ofstream out_str(argv[2]);
    if (!out_str.good()) {
        std::cerr << "Can not open" <<argv[2] << " to write." << std::endl;
    }

    //read the processes from file
    std::vector<Process> p_container;
    while (1) {
    	Process p;
    	if (p.read(in_str)) {
    		p_container.push_back(p);
    	} else {
    		break;
    	}
    }
    in_str.close();

    OS myos(p_container, 1, 8); // 1 processor, context switch time = 8 ms

    myos.schedule(OS::FCFS_sort, "FCFS");
    std::cout << std::endl;
    myos.report_result(std::string(argv[2]), "FCFS");
    myos.reset();

    myos.schedule(OS::SJF_sort, "SJF");
    std::cout << std::endl;
    myos.report_result(std::string(argv[2]), "SJF");
    myos.reset();

    OS myos2(p_container,1, 8, 84); // 1 processor, context switch time = 8 ms, time slice = 84 ms
    myos2.schedule(OS::FCFS_sort, "RR");
    myos2.report_result(std::string(argv[2]), "RR");
    myos.reset();
    return 0;
}
