#include "SimJoiner.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <sstream>
#include <string>
#include <cmath>

#define MAX_LEN 2048
#define MAX 0x7FFFFFFF
using namespace std;


vector<string> entries;
unordered_map<int,int> candidates;
unordered_map<string, VectorItem> gram_indexed;
vector<vector<string>> file2_grams;
unsigned int calc_hash(const string &s, int start, int end) {
	unsigned int seed = 131;
	unsigned int hash = 0;
	//string st;
	for(int i=start; i<end; i++)
	{
		hash = hash*seed+s[i];
		//st+=s[i];
	}
	//cout << "s: "<< st << " hash: "<< (hash & MAX) <<endl;
	return (hash & MAX);
}
int maxv(int a, int b, int c) {
	int t = a>b?a:b;
	return t>c?t:c;
}
int minv(int a,int b, int c) {
	int t = a>b?b:a;
	return t>c?c:t;  
}
int min(int a,int b) {
	return a>b?b:a;
}
int max(int a,int b) {
	return a>b?a:b;
}
SimJoiner::SimJoiner()
{
}

SimJoiner::~SimJoiner()
{
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, unsigned q, double threshold, vector<JaccardJoinResult> &result)
{
	//cout << "-----------------" << threshold << endl;
	gram_indexed.clear();
	entries.clear();
	candidates.clear();
	file2_grams.clear();
	//cout << "prefix num: " << prefix_num << endl;
	unordered_map<string, unsigned> all_gram_frequency;
	unordered_map<string, unsigned>  str_gram_frequency;
	result.clear();
	ifstream fin(filename2);
	string str,str1,sub,sub1;
	int t=0, len=0;
	if (!fin)
		return false;
	while(getline(fin, str1)){
		str_gram_frequency.clear();
		len=str1.length();
		vector<string> str_gram;
		str="";t=0;
		while(t<len)
		{	
			if(str1[t] == '\n' || str1[t]=='\r')
				break;
			else
				str+=str1[t];
			t++;
		}
		len=str.length();
		for(int pos =0 ; pos <= max(len-q,0); pos++ ) {
			sub = str.substr(pos, q);
			if (str_gram_frequency.find(sub) != str_gram_frequency.end()) {
				str_gram_frequency[sub]+=1;
				stringstream ss;
				ss << str_gram_frequency[sub];
				sub1 = sub + ss.str();
				str_gram.push_back(sub1);
				if (all_gram_frequency.find(sub1) != all_gram_frequency.end()) {
					all_gram_frequency[sub1]+=1;
				} else {
					all_gram_frequency.insert(make_pair(sub1,1));
				}
			} else {
				str_gram_frequency.insert(make_pair(sub,0));
				if (all_gram_frequency.find(sub) != all_gram_frequency.end()) {
					all_gram_frequency[sub]+=1;
				} else {
					all_gram_frequency.insert(make_pair(sub,1));
				}
				str_gram.push_back(sub);
			}
		}
		file2_grams.push_back(str_gram);
	}
	/*for (auto& sub_num:all_gram_frequency) {
		cout << sub_num.first << "  " << sub_num.second << endl;
	}*/
	fin.close();
	vector<PAIR> find_prefix_grams;
	for (int index = 0; index < file2_grams.size(); index++) {
		vector<string> str = file2_grams[index];
		find_prefix_grams.clear();
		for (auto sub_str:str) {
			find_prefix_grams.push_back(make_pair(sub_str,all_gram_frequency[sub_str]));
		}
		sort(find_prefix_grams.begin(), find_prefix_grams.end(), 
		[](const PAIR& a, const PAIR& b){ return a.second < b.second; });
		int prefix_num = ceil(find_prefix_grams.size() * (1-threshold) +1);
		int real_prefix_num = min(prefix_num, find_prefix_grams.size());
		for (int i = 0; i < real_prefix_num; ++i)
		{
			if (gram_indexed.find(find_prefix_grams[i].first) != gram_indexed.end()) {
				vector<int>& v=gram_indexed[find_prefix_grams[i].first].getVector();
				v.push_back(index);
			} else {
				VectorItem ids;
				ids.insert(index);
				gram_indexed.insert(make_pair(find_prefix_grams[i].first, ids));
			}
		}
	}
	/*for (auto & ii:gram_indexed) {
		cout << "------------" << ii.first << endl;
		VectorItem vtemp= ii.second;
		vector<int>& v = vtemp.getVector(); 
		for(auto &j:v) {
			cout << j << " " ;
		}
		cout << endl;
	}*/
	
	ifstream fin2(filename1);
	int index =0;
	while(getline(fin2, str1)) {
		str_gram_frequency.clear();
		find_prefix_grams.clear();
		candidates.clear();
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
		len=str.length();
		for(int pos =0 ; pos <= max(len-q,0); pos++ ) {
			sub = str.substr(pos, q);
			if (str_gram_frequency.find(sub) != str_gram_frequency.end()) {
				str_gram_frequency[sub]+=1;
				stringstream ss;
				ss << str_gram_frequency[sub];
				sub1 = sub + ss.str();
				find_prefix_grams.push_back(make_pair(sub1,all_gram_frequency[sub1]));
			} else {
				str_gram_frequency.insert(make_pair(sub,0));
				find_prefix_grams.push_back(make_pair(sub,all_gram_frequency[sub]));
			}
		}
		sort(find_prefix_grams.begin(), find_prefix_grams.end(), 
		[](const PAIR& a, const PAIR& b){ return a.second < b.second; });
		/*for (auto& i:find_prefix_grams) {
			cout << "   " << i.first << "  " << i.second << endl;
		}*/
		int prefix_num = ceil(find_prefix_grams.size() * (1-threshold) +1);
		int real_prefix_num = min(prefix_num, find_prefix_grams.size());
		for (int i = 0; i < real_prefix_num; ++i)
		{
			if (gram_indexed.find(find_prefix_grams[i].first) != gram_indexed.end()) {
				vector<int>& v=gram_indexed[find_prefix_grams[i].first].getVector();
				for (int vt=0; vt<v.size(); vt++) {
					if (candidates.find(v[vt])==candidates.end())
						candidates.insert(make_pair(v[vt],0));
				}
			}
		}
		for(auto & can:candidates) {
			//cout << "pair " << can.first << "   " << can.second <<endl;
			double val = getJaccardDistance(file2_grams[can.first],find_prefix_grams, threshold);
			//cout << "val---------" << val << endl;
			if (val > threshold) {
				result.push_back(JaccardJoinResult(index, can.first, val));
			}
		}
		index+=1;
	}
	sort(result.begin(), result.end(), 
		[](const JaccardJoinResult& a, const JaccardJoinResult& b){ return a.id1 < b.id1 || (a.id1 == b.id1 && a.id2 < b.id2); }
	);
	/*for (auto& entry :result) {
		cout << entry.id1 << "  " << entry.id2 << "  " << entry.s << endl;
	}*/
	return SUCCESS;
}

double SimJoiner::getJaccardDistance(vector<string>& grams1, vector<PAIR>& grams2,double threshold) {
	int len1 = grams1.size(), len2 = grams2.size();
	/*cout << len1 << "  " << len2 << "  " << threshold << endl;
	int min_com = ceil(threshold*(len1+len2)/(1+threshold));
	return min_com*1.0/(len1+len2-min_com);*/
	int common=0;
	map<string, int> temp;
	if (len1 > len2) {
		temp.clear();
		for (auto gram:grams1) {
			temp.insert(make_pair(gram, 0));
		}
		for (auto gram:grams2) {
			if (temp.find(gram.first)!=temp.end()) {
				common++;
			}
		}
	} else {
		temp.clear();
		for (auto gram:grams2) {
			temp.insert(make_pair(gram.first, 0));
		}
		for (auto gram:grams1) {
			if (temp.find(gram)!=temp.end()) {
				common++;
			}
		}
	}
	return common*1.0/(len1+len2-common);
}
int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned q, unsigned threshold, vector<EDJoinResult> &result)
{
	result.clear();
	vector<vector<string>> data1;
	vector<vector<int>> data1_num;
	vector<vector<int>> data2_num;
	vector<vector<string>> data2;

	vector<vector<vector<int>>> data2_info;
	readData(filename1, data1,data1_num);
	readData2(filename2, data2_info,data2,data2_num,threshold);
	int i=0,j=0,p =0,k=0,cal_val=0, li=0,pos=0,left=0,right=0,x=0;
	bool flag = false;
	int hashx[2048];
	
	for (i =0; i <data1.size(); i++) {
		vector<string>& datas = data1[i];
		for (int q = 0; q <datas.size(); q++) {
			j= max(0,i-(int)threshold);
			for(; j<=i+(int)threshold; j++) {
				vector<vector<int>>& data2_sub = data2_info[j];
				if(data2_sub.size()!=0) {
					li = j/(threshold+1);
					for (x = 0; x <= i-li; x++) hashx[x] = calc_hash(datas[q],x,x+li);
					for (p=0; p<data2_sub.size();p++) {
						flag = false;
						for(k =0; k<=threshold; k++){
						 	pos = j*k/(threshold+1);
						 	left = maxv(0,pos-k, pos-(threshold-k)+(i-j));
						 	right = minv(i-li,pos+k,pos+(threshold-k)+(i-j));
							for(x = left; x <=right; x++) {
								if (hashx[x]==data2_info[j][p][k]) {
									flag = true;
									break;
								}
							}	
							if(flag==true) {
								cal_val = getEditDistance(datas[q],data2[j][p],i,j,threshold);
								if ( cal_val <= threshold) {
									result.push_back(EDJoinResult(data1_num[i][q], data2_num[j][p], cal_val));
								}
								break;
							}
						}
					}	
				
				}
			}
		}
	}     
	/*for (auto& entry :result) {
		cout << entry.id1 << "  " << entry.id2 << "  " << entry.s << endl;
	}*/
	sort(result.begin(), result.end(), 
		[](const EDJoinResult& a, const EDJoinResult& b){ return a.id1 < b.id1 || (a.id1 == b.id1 && a.id2 < b.id2); }
	);
	return SUCCESS;
}

bool SimJoiner::readData(const char * filename, vector<vector<string>>& data, vector<vector<int>>& data_num){
	ifstream fin(filename);
	string str1,str;
	int t=0;
	if (!fin)
		return false;
	int line_num=0,len=0;
	vector<string> vec;
	vector<int> vec_num;
	for (int i = 0; i < MAX_LEN; i++) {
		data.push_back(vec);
		data_num.push_back(vec_num);
	}

	while(getline(fin, str1)){
		str="";
		t=0;
		len=str1.length();
		while(t<len)
		{	
			if(str1[t] == '\n' || str1[t]=='\r')
				break;
			else
				str+=str1[t];
			t++;
		}
		len=str.length();
		vector<string>& vec1 = data[len];
		vector<int>& vec1_num = data_num[len];
		vec1.push_back(str);
		vec1_num.push_back(line_num);
		line_num++;
	}	
	fin.close();
	return true;
}	

bool SimJoiner::readData2(const char * filename,vector<vector<vector<int>>>& datainfo,vector<vector<string>>& data,vector<vector<int>>& data_num,unsigned threshold){
	ifstream fin(filename);
	string str,str1;
	int t=0;
	if (!fin)
		return false;
	int line_num=0,len=0;
	vector<int> vec;
	vector<string> vecs;
	vector<vector<int>> vect; 
	for (int i = 0; i < MAX_LEN; i++) {
		data_num.push_back(vec);
		data.push_back(vecs);
		datainfo.push_back(vect);	
	}
	while(getline(fin, str1)){
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
		len=str.length();
		vector<int>& vec1 = data_num[len];
		vector<string>& vecs1 = data[len];
		vec1.push_back(line_num);
		vecs1.push_back(str);
		vector<vector<int>>& vec2 = datainfo[len];
		vector<int> vect2;
		for (int i = 0; i<=threshold; i++) {
			//vect2.push_back(calc_hash(str.substr(len/(threshold+1)*i, len/(threshold+1))));
			vect2.push_back(calc_hash(str,len*i/(threshold+1), len*i/(threshold+1) + len/(threshold+1)));			
		}
		vec2.push_back(vect2);
		line_num++;
	}	
	fin.close();
	return true;
}	

int SimJoiner::getEditDistance(const string &A, const string &B, int n, int m, unsigned threshold) {
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
