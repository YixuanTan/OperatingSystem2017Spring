//
//  OS2017
//  project2
//
//

#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;
vector<string> memo;
int numOfFrame, framePerRow, numOfRows;

void readInput() {
    
}

void output() {
    
    cout << string(numOfFrame, '=') << endl;
    for(int i = 0; i < numOfRows; i++) {
        cout << memo[i] << endl;
    }
    cout << string(numOfFrame, '=') << endl;
}
int main(int argc, const char * argv[]) {
    // insert code here...
    numOfFrame = 256;
    framePerRow = 32;
    numOfRows = numOfFrame / framePerRow;
    
    // initialization
    memo.resize(numOfRows, string(numOfFrame, '.'));
    
    readInput(argv[1]);
    
    return 0;
}
