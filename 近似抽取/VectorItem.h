#ifndef VECTORITEM_H
#define VECTORITEM_H


#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

class VectorItem
{
	public:
		vector<int> ivector;
		VectorItem();
		~VectorItem();
		void insert(int no);
		vector<int> & getVector();
		void sort();
		int size() const;
};

#endif
