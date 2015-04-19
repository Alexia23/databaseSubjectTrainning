#pragma once
#include <vector>
#include <string>
#include <map>
#include "VectorItem.h"

using namespace std;
typedef pair<string, int> PAIR;
template <typename _IDType, typename _SimType>
struct JoinResult
{
	JoinResult(_IDType id11, _IDType id21, _SimType s1) : id1(id11), id2(id21), s(s1) {}
	_IDType id1;
	_IDType id2;
	_SimType s;
};

typedef JoinResult<unsigned, double> JaccardJoinResult;
typedef JoinResult<unsigned, unsigned> EDJoinResult;

const int SUCCESS = 0;
const int FAILURE = 1;

class SimJoiner
{
public:
	SimJoiner();
	~SimJoiner();
	bool readData(const char * filename, vector<vector<string>>& data, vector<vector<int>>& data_num);
	bool readData2(const char * filename,vector<vector<vector<int>>>& datainfo,vector<vector<string>>& data,vector<vector<int>>& data_num,unsigned threshold);
	int joinJaccard(const char *filename1, const char *filename2, unsigned q, double threshold, std::vector<JaccardJoinResult> &result);
	int joinED(const char *filename1, const char *filename2, unsigned q, unsigned threshold, std::vector<EDJoinResult> &result);
	int getEditDistance(const string &A, const string &B, int n, int m, unsigned threshold);
	double getJaccardDistance(vector<string>& grams1, vector<PAIR>& grams2,double threshold);
};

