#ifndef SMALLHEAP_H
#define SMALLHEAP_H


#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;
typedef pair<int, int> element;
class SmallHeap
{
	public:
		int heapsize = 0;
		vector<element> queue;
		void add (element e) {
			shiftup(heapsize++, e);
		}
		int size(){
			return heapsize;
		}
		void shiftup(int s, element e) {
			queue.push_back(e);
			while(s > 0) {
				int parent = (s-1)/2;
				if (queue[parent].first < e.first) {
					break;
				}
				queue[s] = queue[parent];
				s = parent;
			}
			queue[s] = e;
		}
		element poll() {
			element ret = queue[0];  // top
			int s = --heapsize;
			shiftdown(0, queue[s]);  
			return ret;
		}
		void shiftdown(int i ,element e) {
			int half = heapsize /2;
			while(i <half) {
				int child = 2*i+1;
				int right = child+1;
				if(right<heapsize && queue[child].first > queue[right].first) {
					child = right;
				}
				if (e.first < queue[child].first) {
					break;
				}
				queue[i] = queue[child];
				i = child;
			}
			queue[i] =e;
		}

};

#endif
