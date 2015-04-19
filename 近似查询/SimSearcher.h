#pragma once
#include <vector>
#include <utility>
#include "VectorItem.h"
#include "SmallHeap.h"

using namespace std;

const int SUCCESS = 0;
const int FAILURE = 1;

class SimSearcher
{
public:
	SimSearcher();
	~SimSearcher();
	unordered_map<string, VectorItem> gram_indexed;
	vector<string> id_str;
	vector<int> less_q_ids;
	unsigned qv;
	int createIndex(const char *filename, unsigned q);
	int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double> > &result);
	int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned> > &result);
	int getEditDistance(const string &A, const string &B, int n, int m, unsigned threshold);
	double getJaccardDistance(const string &A, const string&B, double threshold);
};

