#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <cstring>
#include <fstream>
#include <cmath>
#include <ctime>
#include "VectorItem.h"

using namespace std;

template <typename _IDType, typename _PosType, typename _LenType, typename _SimType>
struct ExtractResult
{
	_IDType id;
	_PosType pos;
	_LenType len;
	_SimType sim;
};

typedef ExtractResult<unsigned, unsigned, unsigned, unsigned> EDExtractResult;
typedef ExtractResult<unsigned, unsigned, unsigned, double> JaccardExtractResult;

const int SUCCESS = 0;
const int FAILURE = 1;

class AEE {
public:
	vector<string> entities;
	unordered_map<string, VectorItem> inverted_lists;
	vector<vector<int>> entities_pos;
	AEE();
	~AEE();

	int createIndex(const char *filename, unsigned q);
	int aeeJaccard(const char *doc, double threshold, std::vector<JaccardExtractResult> &result);
	int aeeED(const char *doc, unsigned threshold, std::vector<EDExtractResult> &result);

	int binaryShift(int i ,int j, int tl, int te, vector<int>& entity_pos);
	int binarySpan(int i , int j, int te, int dte, vector<int>& entity_pos,int id, 
		int entirylen,std::vector<EDExtractResult> &candidate);
	int binarySpan1(int i , int j, int te, int dte, vector<int>& entity_pos,int id, 
		int entirylen,std::vector<JaccardExtractResult> &candidate);
	int getEditDistance(const string &A, const string &B, int n, int m, unsigned threshold);
	double getJaccardDistance(const string &A, const string &B, int n, int m, double threshold, int t);
};
