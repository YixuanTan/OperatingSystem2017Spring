#include <iostream>
#include <fstream>
#include "OS.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "ERROR: Invalid arguments" << std::endl;
        std::cout << "USAGE: ./a.out <input-file>" << std::endl;
        return 1;
    }
    std::ifstream infile;
    infile.open(argv[1]);
    OS myos(infile, 256);
    myos.schedule(0);
    std::cout << std::endl;
    
    infile.open(argv[1]);
    OS myos1(infile, 256);
    myos1.schedule(1);
    std::cout << std::endl;

    infile.open(argv[1]);
    OS myos2(infile, 256);
    myos2.schedule(2);
    std::cout << std::endl;

    infile.open(argv[1]);
    OS myos3(infile, 256);
    myos3.schedule(3);
    
    return 0;
}
