#include "AEE.h"

using namespace std;
int qv=0;

AEE::AEE()
{
}

AEE::~AEE()
{
}

int AEE::createIndex(const char *filename, unsigned q)
{
	qv=q;
	entities.clear();
	ifstream fin(filename);
    string s, subs;
	int len;
	int num = 0;
    while (getline(fin,s)) {
			entities.push_back(s);
	        len = int(s.length())-q;
	        vector<int> vl(1001);
		 	entities_pos.push_back(vl);	
		for ( int i = 0; i <= len; i++ ) {
			subs=s.substr(i, q); 
			if(inverted_lists.find(subs)==inverted_lists.end()){
				 VectorItem entity_list;
				 inverted_lists[subs]=entity_list;
			}
			inverted_lists[subs].insert(num);
		}	
		num++;
		//cout << s << endl;
	}
	for (auto & i : inverted_lists)
	{
		i.second.sort();
	}
 //    unordered_map<string, VectorItem>::iterator mapv;	
	// for (mapv = inverted_lists.begin(); mapv!=inverted_lists.end(); mapv++) {
	// 	cout << mapv->first <<endl;
	// 	VectorItem vp = mapv->second;
	// 	vector<int>& v=vp.getVector();
	// 	for(int i = 0; i <v.size(); i++ ){
	// 		 cout << v[i] << " ";
	// 	}
	// 	cout << endl;
	// }
	// cout << entities.size()<<endl;
	if (inverted_lists.empty())
		return FAILURE;
	return SUCCESS;
}

int AEE::aeeJaccard(const char *doc, double threshold, vector<JaccardExtractResult> &result)
{
	result.clear();
	string s(doc);
	string sub;
	for (auto & sl : entities_pos)
		sl.clear();
	int len = s.length();
	for(int pos =0 ; pos <= len-qv ; pos++ ) {
		sub = s.substr(pos, qv);
		if (inverted_lists.find(sub) != inverted_lists.end()) {
			vector<int>& v=inverted_lists[sub].getVector();
			for(int i = 0; i <v.size(); i++ ){
			 entities_pos[v[i]].push_back(pos);
			}
		}
	}
	std::vector<JaccardExtractResult> candidate;
	int index = 0;
	for(auto & entity_pos : entities_pos) {
		if(entities_pos.empty())
			continue;
		int e = int(entities[index].length()) - qv+1;
		int tl = max(int(ceil(e * threshold)), 0);
		int te = max(int(floor(e/threshold)), 0);
		int dte = max(int(ceil(e * threshold)), 0); 
		if (entity_pos.size() >= tl) {
			int i = 0; 
			while (i<=entity_pos.size()-tl) {
				int j = i+tl-1;
				if (entity_pos[j] - entity_pos[i]+1 <= te) {
					//binarySpan
					binarySpan1(i , j, te, dte, entity_pos,index, len, candidate);
					i++;
				} else {
					//binaryshift
					i = binaryShift(i, j, tl, te, entity_pos);
				}
			}
		}
		index ++;
	}
	JaccardExtractResult oner;
	//cout << candidate.size() << endl;
	string sub_doc; 
	for (auto & can :candidate) {
		sub_doc = s.substr(can.pos, can.len);
		double distance = getJaccardDistance(sub_doc,entities[can.id], sub_doc.length(), 
			entities[can.id].length(), threshold, can.sim);
		if (distance >= threshold) {
			oner.id = can.id;
			oner.pos = can.pos;
			oner.len = can.len;
			oner.sim = distance;
			result.push_back(oner);
		}
	}
	return SUCCESS;
}

int AEE::aeeED(const char *doc, unsigned threshold, vector<EDExtractResult> &result)
{
	result.clear();
	string s(doc);
	string sub;
	for (auto & sl : entities_pos)
		sl.clear();
	int len = s.length();
	for(int pos =0 ; pos <= len-qv ; pos++ ) {
		sub = s.substr(pos, qv);
		if (inverted_lists.find(sub) != inverted_lists.end()) {
			vector<int>& v=inverted_lists[sub].getVector();
			for(int i = 0; i <v.size(); i++ ){
			 entities_pos[v[i]].push_back(pos);
			}
		}
	}
	// for(int i = 0; i < entities_pos.size(); i++) {
	// 	vector<int>& v = entities_pos[i];
	// 	for(int j = 0 ; j <v.size(); j++)
	// 		cout  << v[j] << " ";
	// 	cout << endl;
	// }
	//cout << s << endl;
	std::vector<EDExtractResult> candidate;
	int index = 0;
	for(auto & entity_pos : entities_pos) {
		if(entities_pos.empty())
			continue;
		int e = int(entities[index].length()) - qv+1;
		int tl = max(e - int(threshold)*qv, 0);
		int te = max(e + int(threshold), 0);
		int dte = max(e - int(threshold) , 0); 
		if (entity_pos.size() >= tl) {
			int i = 0; 
			while (i<=entity_pos.size()-tl) {
				int j = i+tl-1;
				if (entity_pos[j] - entity_pos[i]+1 <= te) {
					//binarySpan
					binarySpan(i , j, te, dte, entity_pos,index, len, candidate);
					i++;
				} else {
					//binaryshift
					i = binaryShift(i, j, tl, te, entity_pos);
				}
			}
		}
		index ++;
	}
	EDExtractResult oner;
	//cout << candidate.size() << endl;
	string sub_doc; 
	for (auto & can :candidate) {
		sub_doc = s.substr(can.pos, can.len);
		int distance = getEditDistance(sub_doc,entities[can.id], sub_doc.length(), 
			entities[can.id].length(), threshold);
		if (distance <= threshold) {
			oner.id = can.id;
			oner.pos = can.pos;
			oner.len = can.len;
			oner.sim = distance;
			result.push_back(oner);
		}
	}
	return SUCCESS;
}
int AEE::binaryShift(int i ,int j, int tl, int te, vector<int>& entity_pos) {
	int lower = i, upper = j, mid = 0;
	while (lower <= upper) {
		mid = (lower + upper+1) >> 1;
		if (entity_pos[j] +(mid-i)-entity_pos[mid]+1 > te) {
			lower = mid+1;
		} else {
			upper = mid-1;
		}
	}
	i = lower,j=i+tl-1;
	if(j >=int(entity_pos.size()))
		return i;
	if (entity_pos[j] - entity_pos[i] +1 > te) {
		i = binaryShift(i,j,tl,te,entity_pos);
	} else {
		return i;
	}
}
int AEE::binarySpan(int i , int j, int te, int dte, vector<int>& entity_pos,int id, 
		int entitylen, std::vector<EDExtractResult> &candidate) {

	int lower = j, upper = min(i+te-1, int(entity_pos.size())-1), mid = 0;
	while (lower <= upper) {
		mid = (lower + upper+1) >> 1;
		if (entity_pos[mid] -entity_pos[i]+1 > te) {
			upper = mid-1;
		} else {
			lower =mid+1;
		}
	}
	mid = upper;
	int spos = 0;
	if (i > 0) 
		spos = max(spos, entity_pos[i-1] +1);
	EDExtractResult can;
	for (int start = spos; start <= entity_pos[i]; start++) {
		int minv = start + dte -1;
		int maxv = min(start+te-1, entitylen-qv);
		for (int v = minv; v <= maxv; v++) {
			can.id = id;
			can.pos = start;
			can.len = v + qv - start;
			can.sim = mid - i + 1;
			candidate.push_back(can);
		}
	}
}
int AEE::binarySpan1(int i , int j, int te, int dte, vector<int>& entity_pos,int id, 
		int entitylen, std::vector<JaccardExtractResult> &candidate) {

	int lower = j, upper = min(i+te-1, int(entity_pos.size())-1), mid = 0;
	while (lower <= upper) {
		mid = (lower + upper+1) >> 1;
		if (entity_pos[mid] -entity_pos[i]+1 > te) {
			upper = mid-1;
		} else {
			lower =mid+1;
		}
	}
	mid = upper;
	int spos = 0;
	if (i > 0) 
		spos = max(spos, entity_pos[i-1] +1);
	JaccardExtractResult can;
	for (int start = spos; start <= entity_pos[i]; start++) {
		int minv = start + dte -1;
		int maxv = min(start+te-1, entitylen-qv);
		for (int v = minv; v <= maxv; v++) {
			can.id = id;
			can.pos = start;
			can.len = v + qv - start;
			can.sim = mid - i + 1;
			candidate.push_back(can);
		}
	}
}
int AEE::getEditDistance(const string &A, const string &B, int n, int m, unsigned threshold) {
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
double AEE::getJaccardDistance(const string &A, const string &B, int n, int m, double threshold, int t) {
	double dis = 0;
	if (m > n) {
		if (n < qv) {
				return dis;
		} else {
			dis = (double)t / (n + m - 2 * (qv - 1) - t);
			return dis;
		}
	} else {
		getJaccardDistance(B, A,m,n,threshold,t);
	}
}