#include "SimSearcher.h"
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <time.h>

using namespace std;


#define MIU 0.0085
typedef pair<string, VectorItem> PAIR;
typedef pair<int, int> result_element;
typedef pair<int, double>  result_jaccard;



int min(int a,int b) {
	return a>b?b:a;
}
int max(int a,int b) {
	return a>b?a:b;
}
int BinSearch(vector<int>& v, int small_value) {
	int low = 0, high = v.size()-1, mid;
	while(low <= high) {
		mid = (low+high)/2;
		if(v[mid]==small_value)
			return mid;
		else if(v[mid]>small_value) {
			if ((mid-1>=0) && v[mid-1]<small_value)
				return mid;
			else if ((mid-1>=0) && v[mid-1]==small_value)
				return mid-1;
			else
				high = mid -1;
		} else {
			low = mid +1;
		}
	}
	return -1;
}
int BinSearchValue(vector<int>& v, int small_value) {
	int low = 0, high = v.size()-1, mid;
	while(low <= high) {
		mid = (low+high)/2;
		if(v[mid]==small_value)
			return mid;
		else if(v[mid]>small_value) {
			high = mid -1;
		} else {
			low = mid +1;
		}
	}
	return -1;
}
SimSearcher::SimSearcher()
{
}

SimSearcher::~SimSearcher()
{
}


int SimSearcher::createIndex(const char *filename, unsigned q)
{
	id_str.clear();
	gram_indexed.clear();
	less_q_ids.clear();
	qv = q;
	unordered_map<string, int>  str_gram_frequency;
	ifstream fin(filename);
	string str,str1,sub,sub1;
	int t=0, len=0;
	if (!fin)
		return false;
	int index = 0;
	while(getline(fin, str1)){
		str_gram_frequency.clear();
		len=str1.length();
		str="";t=0;
		while(t<len)
		{	
			if(str1[t] == '\n' || str1[t]=='\r')
				break;
			else
				str+=str1[t];
			t++;
		}
		len = str.length();
		if (len < q) 
			less_q_ids.push_back(index);
		else {
			for(int pos =0 ; pos <= len-q; pos++ ) {
				sub = str.substr(pos, q);
				if (str_gram_frequency.find(sub) != str_gram_frequency.end()) {
					str_gram_frequency[sub]+=1;
					stringstream ss;
					ss << str_gram_frequency[sub];
					sub1 = sub + ss.str();
					if (gram_indexed.find(sub1) != gram_indexed.end()) {
						vector<int>& v=gram_indexed[sub1].getVector();
						v.push_back(index);
					} else {
						VectorItem ids;
						ids.insert(index);
						gram_indexed.insert(make_pair(sub1, ids));
					}
				} else {
					str_gram_frequency.insert(make_pair(sub, 0));
					if (gram_indexed.find(sub) != gram_indexed.end()) {
						vector<int>& v=gram_indexed[sub].getVector();
						v.push_back(index);
					} else {
						VectorItem ids;
						ids.insert(index);
						gram_indexed.insert(make_pair(sub, ids));
					}
				}
			}
		}
		id_str.push_back(str);
		index++;
	}
	fin.close();
	return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();
	vector<string> sub_query;
	unordered_map<string, int>  str_gram_frequency;
	string str_query(query), sub,sub1;
	vector<PAIR> grams_query;
	vector<int> pop_index;
	unordered_map<int, int> can;
	vector<pair<int,int>> pop_lists;
	int n = 0;
	int m = 0, l = 0, t =0;
	int q_gram_num = max(0,(str_query.length()-qv+1));

	int min_length=id_str[0].length();
	for (int i = 1; i < id_str.size(); ++i)
	{
		if (id_str[i].length() < min_length)
			min_length = id_str[i].length();
	}
	int shortest_gram_num =max(0,min_length-qv+1);
	t = max(threshold*q_gram_num,
		(q_gram_num+shortest_gram_num)*threshold/(1+threshold));
	if (t <= 0) {
		for (int i = 0; i < id_str.size(); i++) {
			double dis = getJaccardDistance(str_query, id_str[i],threshold);
			if (dis >= threshold) {
				result.push_back(make_pair(i, dis));
			}
		}
		return SUCCESS;
	}
	for(int pos =0 ; pos < max(str_query.length()-qv+1,0); pos++ ) {
		sub = str_query.substr(pos, qv);
		if (str_gram_frequency.find(sub) != str_gram_frequency.end()) {
			str_gram_frequency[sub]+=1;
			stringstream ss;
			ss << str_gram_frequency[sub];
			sub1 = sub + ss.str();
			sub_query.push_back(sub1);
		} else {
			sub_query.push_back(sub);
			str_gram_frequency.insert(make_pair(sub, 0));
		}
	}

	for(auto& j:sub_query) {
		if (gram_indexed.find(j)!= gram_indexed.end())
			grams_query.push_back(make_pair(j,gram_indexed[j]));
	}
	sort(grams_query.begin(), grams_query.end(),
		[](const PAIR& a, const PAIR& b){ return a.second.size() < b.second.size(); });
	//long long start= clock();
	if (grams_query.size() > 0) {
		m = grams_query[grams_query.size()-1].second.size();
		l = t*1.0/(MIU*log10(m)+1);
	}
	if (l <= 0  || grams_query.size() <= l){
		for (int i = 0; i < id_str.size(); i++) {
			double dis = getJaccardDistance(str_query, id_str[i],threshold);
			if (dis >= threshold) {
				result.push_back(make_pair(i, dis));
			}
		}
		return SUCCESS;
	}
	SmallHeap hs;
	for (int i = 0; i < grams_query.size()-l; ++i)
	{
		pop_index.push_back(0);
		vector<int> temp = grams_query[i].second.getVector();
		hs.add(make_pair(temp[pop_index[i]], i));
	}
	while(hs.size()!=0) {
		pop_lists.clear();
		pair<int,int> j = hs.queue[0];  //id, gram_num
		n = 0;
		while( (hs.size()!=0) && (j.first == hs.queue[0].first) ) {
			n++;
			pair<int,int> poll_value = hs.poll();
			pop_lists.push_back(poll_value);
			pop_index[poll_value.second]++;
		}
		if (n >= t-l) {
			if (can.find(j.first)!=can.end()) {
				can[j.first] = n;
			} else {
				can.insert(make_pair(j.first, n));
			}
			for (int i = 0; i < pop_lists.size(); ++i)
			{
				int gram_num = pop_lists[i].second;
				vector<int> temp = grams_query[gram_num].second.getVector();
				if (pop_index[gram_num] < temp.size())
					hs.add(make_pair(temp[pop_index[gram_num]],gram_num));
			}
		} else {
			for (int nv = 0; nv< t-l-n; ++nv)
			{
				if (hs.size()==0)
					break;
				pair<int,int> poll_value = hs.poll();
				pop_lists.push_back(poll_value);
				pop_index[poll_value.second]++;
			}
			if (hs.size()!=0) {
				int new_t = hs.queue[0].first;
				for (int i = 0; i < pop_lists.size(); ++i)
				{
					int gram_num = pop_lists[i].second;
					vector<int>& temp = grams_query[gram_num].second.getVector();
					int index = BinSearch(temp, new_t);
					if (index == -1) {
						pop_index[gram_num] = temp.size();
					} else {
						pop_index[gram_num] = index;
						hs.add(make_pair(temp[index],gram_num));
					}
				}
			}
		}
	}
	for(auto & p:can) {
		for (int i = grams_query.size()-l; i < grams_query.size(); ++i)
		{
			vector<int>& temp = grams_query[i].second.getVector();
			if (BinSearchValue(temp, p.first)!=-1) {
				p.second++;
			}
		}
	}
	//printf("find value: %dms\n", (clock() - end1) / (CLOCKS_PER_SEC / 1000));
	vector<int>  final_can;
	for(auto& p:can) {
		if (p.second > t) {
			final_can.push_back(p.first);
		}
	}
	for(auto&p:less_q_ids){
		final_can.push_back(p);
	}
	for(auto & p:final_can) {
		double dis = getJaccardDistance(str_query, id_str[p], threshold);
		if (dis >= threshold) {
			result.push_back(make_pair(p, dis));
		}
	}
	sort(result.begin(), result.end(),[](const result_jaccard& a, const result_jaccard& b)
		{ return  a.first < b.first;  });
	return SUCCESS;
}

int SimSearcher::searchED(const char *query, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
	result.clear();
	vector<string> sub_query;
	unordered_map<string, int>  str_gram_frequency;
	string str_query(query), sub,sub1;
	vector<PAIR> grams_query;
	vector<int> pop_index;
	unordered_map<int, int> can;
	vector<pair<int,int>> pop_lists;
	int n = 0;
	int m = 0, l = 0, t =0;

	t = strlen(query)-qv+1-threshold*qv;
	if (t <= 0) {
		for (int i = 0; i < id_str.size(); i++) {
			int dis = getEditDistance(str_query, id_str[i], str_query.length(), id_str[i].length(), threshold);
			if (dis <= threshold) {
				result.push_back(make_pair(i, dis));
			}
		}
		return SUCCESS;
	}
	for(int pos =0 ; pos < max(str_query.length()-qv+1,0); pos++ ) {
		sub = str_query.substr(pos, qv);
		if (str_gram_frequency.find(sub) != str_gram_frequency.end()) {
			str_gram_frequency[sub]+=1;
			stringstream ss;
			ss << str_gram_frequency[sub];
			sub1 = sub + ss.str();
			sub_query.push_back(sub1);
		} else {
			sub_query.push_back(sub);
			str_gram_frequency.insert(make_pair(sub, 0));
		}
	}

	for(auto& j:sub_query) {
		if (gram_indexed.find(j)!= gram_indexed.end())
			grams_query.push_back(make_pair(j,gram_indexed[j]));
	}
	sort(grams_query.begin(), grams_query.end(),
		[](const PAIR& a, const PAIR& b){ return a.second.size() < b.second.size(); });
	//long long start= clock();
	if (grams_query.size() > 0) {
		m = grams_query[grams_query.size()-1].second.size();
		l = t*1.0/(MIU*log10(m)+1);
	}
	if (l <= 0  || grams_query.size() < l){
		for (int i = 0; i < id_str.size(); i++) {
			int dis = getEditDistance(str_query, id_str[i], str_query.length(), id_str[i].length(), threshold);
			if (dis <= threshold) {
			result.push_back(make_pair(i, dis));
			}
		}
		return SUCCESS;
	}
	SmallHeap hs;
	for (int i = 0; i < grams_query.size()-l; ++i)
	{
		pop_index.push_back(0);
		vector<int> temp = grams_query[i].second.getVector();
		hs.add(make_pair(temp[pop_index[i]], i));
	}
	while(hs.size()!=0) {
		pop_lists.clear();
		pair<int,int> j = hs.queue[0];  //id, gram_num
		n = 0;
		while( (hs.size()!=0) && (j.first == hs.queue[0].first) ) {
			n++;
			pair<int,int> poll_value = hs.poll();
			pop_lists.push_back(poll_value);
			pop_index[poll_value.second]++;
		}
		if (n >= t-l) {
			if (can.find(j.first)!=can.end()) {
				can[j.first] = n;
			} else {
				can.insert(make_pair(j.first, n));
			}
			for (int i = 0; i < pop_lists.size(); ++i)
			{
				int gram_num = pop_lists[i].second;
				vector<int> temp = grams_query[gram_num].second.getVector();
				if (pop_index[gram_num] < temp.size())
					hs.add(make_pair(temp[pop_index[gram_num]],gram_num));
			}
		} else {
			for (int nv = 0; nv< t-l-n; ++nv)
			{
				if (hs.size()==0)
					break;
				pair<int,int> poll_value = hs.poll();
				pop_lists.push_back(poll_value);
				pop_index[poll_value.second]++;
			}
			if (hs.size()!=0) {
				int new_t = hs.queue[0].first;
				for (int i = 0; i < pop_lists.size(); ++i)
				{
					int gram_num = pop_lists[i].second;
					vector<int>& temp = grams_query[gram_num].second.getVector();
					int index = BinSearch(temp, new_t);
					if (index == -1) {
						pop_index[gram_num] = temp.size();
					} else {
						pop_index[gram_num] = index;
						hs.add(make_pair(temp[index],gram_num));
					}
				}
			}
		}
	}
	for(auto & p:can) {
		for (int i = grams_query.size()-l; i < grams_query.size(); ++i)
		{
			vector<int>& temp = grams_query[i].second.getVector();
			if (BinSearchValue(temp, p.first)!=-1) {
				p.second++;
			}
		}
	}
	/*vector<int>  final_can;
	for(auto& p:can) {
		if (p.second > t) {
			final_can.push_back(p.first);
		}
	}
	for(auto&p:less_q_ids){
		final_can.push_back(p);
	}
	for(auto & p:final_can) {
		int dis = getEditDistance(str_query, id_str[p], str_query.length(), id_str[p].length(), threshold);
		if (dis <= threshold) {
			result.push_back(make_pair(p, dis));
		}
	}*/
	int dis = 0;
	for(auto& p:can) {
		if (p.second >= t) {
			dis = getEditDistance(str_query, id_str[p.first], str_query.length(), id_str[p.first].length(), threshold);
			if (dis <= threshold) {
				result.push_back(make_pair(p.first, dis));
			}
		}
	}
	for(auto&p:less_q_ids){
		dis = getEditDistance(str_query, id_str[p], str_query.length(), id_str[p].length(), threshold);
		if (dis <= threshold) {
			result.push_back(make_pair(p, dis));
		}
	}
	sort(result.begin(), result.end(),[](const result_element& a, const result_element& b)
		{ return  a.first < b.first;  });
	/*for (auto &j:result){
		cout <<id_str[j.first] << "  " << j.second << endl;
	}*/
	return SUCCESS;
}

int SimSearcher::getEditDistance(const string &A, const string &B, int n, int m, unsigned threshold) {
	if (abs(n - m) > threshold) return threshold + 1;
	int V[2][threshold * 2 + 3];
	memset(V, 0x80, sizeof(V));

	V[1][threshold] = 0;
	for (int p = 0; p <= threshold; ++p) {
		int *f = V[p & 1];
		int *g = V[!(p & 1)];
		for (int k = threshold + 1 - p; k <= threshold + 1 + p; ++k) {
			f[k] = max(max(g[k], g[k + 1]) + 1, g[k - 1]);
			int &t = f[k];
			int d = k - threshold - 1;
			if (t >= 0 && t + d >= 0)
				while (t < n && t + d < m && A[t] == B[t + d]) ++t;	
		}
		if (f[m - n + threshold + 1] >= n) return p;
	}
	return threshold + 1;
}

double SimSearcher::getJaccardDistance(const string &A, const string&B, double threshold) {
	//cout << "calc jaccard" << endl;
	int n = A.length(), m = B.length();
	if (n < qv || m < qv )
		return 0.0;
	unordered_map<string, int>  str_gram_frequency;
	unordered_map<string, int> amap;
	unordered_map<string, int> bmap;
	string sub, sub1;
	for(int pos =0 ; pos < max(n-qv+1,0); pos++ ) {
		sub = A.substr(pos, qv);
		if (str_gram_frequency.find(sub) != str_gram_frequency.end()) {
			str_gram_frequency[sub]+=1;
			stringstream ss;
			ss << str_gram_frequency[sub];
			sub1 = sub + ss.str();
			amap.insert(make_pair(sub1, 0));
		} else {
			str_gram_frequency.insert(make_pair(sub, 0));
			amap.insert(make_pair(sub, 0));
		}
	}
	str_gram_frequency.clear();
	for(int pos =0 ; pos < max(m-qv+1,0); pos++ ) {
		sub = B.substr(pos, qv);
		if (str_gram_frequency.find(sub) != str_gram_frequency.end()) {
			str_gram_frequency[sub]+=1;
			stringstream ss;
			ss << str_gram_frequency[sub];
			sub1 = sub + ss.str();
			bmap.insert(make_pair(sub1, 0));
		} else {
			str_gram_frequency.insert(make_pair(sub, 0));
			bmap.insert(make_pair(sub, 0));
		}
	}
	int comm = 0;
	if (amap.size() >= bmap.size()) {
		for(auto& j:bmap) {
			if(amap.find(j.first)!=amap.end())
				comm++;
		}
	} else {
		for(auto& j:amap) {
			if(bmap.find(j.first)!=bmap.end())
				comm++;
		}
	}
	return comm*1.0/(amap.size()+bmap.size()-comm);
}