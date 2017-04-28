//
//  main.cpp
//  sliding_window_median
//
//  Created by Yixuan Tan on 4/27/17.
//  Copyright © 2017 Yixuan Tan. All rights reserved.
//

#include <iostream>
#include <vector>
#include <set>
using namespace std;

int main(int argc, const char * argv[]) {
    // insert code here...
    vector<int> nums = {1,3,-1,-3,5,3,6,7};
    int k = 3;
    int n = nums.size();
    multiset<int> maxheap, minheap;
    int i = 0;
    for(; i < k; i++) maxheap.insert(nums[i]);
    while(minheap.size() < maxheap.size() - 1) {
        minheap.insert(*maxheap.rbegin());
        maxheap.erase(--maxheap.end());
    }
    vector<double> ans;
    for(; i < n; i++) {
        if(minheap.size() == maxheap.size()) ans.push_back(0.5 * (*maxheap.rbegin() + *minheap.begin()));
        else ans.push_back((double)(*maxheap.rbegin()));
        
        // add
        if(nums[i] > *maxheap.rbegin()) minheap.insert(nums[i]);
        else maxheap.insert(nums[i]);
        
        // delete
        if(nums[i - k] > *maxheap.rbegin()) minheap.erase(minheap.lower_bound(nums[i - k]));
        else maxheap.erase(maxheap.lower_bound(nums[i - k]));
        
        // adjust
        if(maxheap.size() > minheap.size() + 1) {
            minheap.insert(*maxheap.rbegin());
            maxheap.erase(--maxheap.end());
        }
        else if(maxheap.size() < minheap.size()) {
            maxheap.insert(*minheap.begin());
            minheap.erase(minheap.begin());
        }
    }
    if(minheap.size() == maxheap.size()) ans.push_back(0.5 * (*maxheap.rbegin() + *minheap.begin()));
    else ans.push_back((double)(*maxheap.rbegin()));

    for(int i : ans) cout << i << endl;
    return 0;
}
