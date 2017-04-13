#include <queue>
#include <vector>
using namespace std;
struct comp{
	virtual void operator()(){}
};

struct bigger : comp {
	bool operator()(const int&a, const int&b){
		return a > b;
	}
};

struct smaller : comp {
	bool operator()(const int&a, const int&b){
		return a < b;
	}
};

struct mycomp {
	bool operator()(const int&a, const int&b){
		return a > b;
	}
};

int main(){
	priority_queue<int> *q1, *q2;

	q1 = new priority_queue<int, vector<int>, bigger>;
	q2 = new priority_queue<int, vector<int>, smaller>;
	return 0;
}